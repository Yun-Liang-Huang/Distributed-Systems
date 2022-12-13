#include "ml_utils.h"

#define SCRIPT_RESULT_BUFFER_SIZE 65536

#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>

#include "../client_request_sender.h"
#include "../filesystem/filesystem_get_request.h"
#include "../filesystem/filesystem_server_context.h"
#include "../filesystem/filesystem_utils.h"
#include "../membership/member_server_context.h"
#include "../membership/member_utils.h"
#include "../utils.h"
#include "ml_run_subjob_request.h"
#include "ml_run_subjob_response.h"
#include "ml_server_context.h"

using namespace std;

string MlUtils::get_ml_output_filename(string job, int number) {
  return job + "_output_" + to_string(number);
}

JobStatus* MlUtils::create_job_status(string job, JobMetadata* jobMetadata) {
  cout << Utils::getTime() << "start create subjobs" << endl;

  JobStatus* jobStatus = new JobStatus();

  string testset = jobMetadata->testset;
  int batch_size = jobMetadata->batch_size;

  // fetch testset file from sdfs
  string temp_file = FileSystemUtils::get_file_temp(testset);

  cout << Utils::getTime() << "temp_file: " << temp_file << endl;

  // split one job into subjobs of batch size
  int total_line_count = Utils::count_file_line(temp_file);

  cout << Utils::getTime() << "total_line_count: " << total_line_count << endl;

  int i = 0;
  while (i < total_line_count) {
    int write_line_count =
        (i + batch_size < total_line_count) ? batch_size : total_line_count - i;
    string new_filename = get_ml_output_filename(job, (i / batch_size) + 1);
    SubJob* subjob = new SubJob(i, write_line_count, new_filename);
    cout << Utils::getTime() << "subjob " << (i / batch_size) + 1 << ": ("
         << subjob->offset << " " << subjob->limit << " "
         << subjob->output_filename << ")" << endl;
    subjob->reset_time();
    jobStatus->todo_subjobs.push(subjob);
    i += write_line_count;
  }

  cout << Utils::getTime()
       << "end create subjobs, queue size: " << jobStatus->todo_subjobs.size()
       << endl;

  if (remove(temp_file.c_str()) == 0) {
    cout << Utils::getTime() << "remove local temp file: " << temp_file
         << " successfully" << endl;
  }

  return jobStatus;
}

void MlUtils::update_ml_worker_pool() {
  // find difference between membership list and all_workers, add those to
  // worker_pool
  set<string> pending_workers;
  MlServerContext::LocalMutex.lock();

  set_difference(
      MemberServerContext::RingMap.begin(), MemberServerContext::RingMap.end(),
      MlServerContext::all_workers.begin(), MlServerContext::all_workers.end(),
      inserter(pending_workers, pending_workers.end()));

  for (auto itr = pending_workers.begin(); itr != pending_workers.end();
       itr++) {
    MlServerContext::worker_pool.push(*itr);
    MlServerContext::all_workers.insert(*itr);
  }

  MlServerContext::LocalMutex.unlock();
}

void MlUtils::reset_ml_worker_pool() {
  MlServerContext::LocalMutex.lock();
  MlServerContext::worker_pool = queue<string>();
  MlServerContext::all_workers = set<string>();
  MlServerContext::LocalMutex.unlock();
}

void MlUtils::finalize_job(string job_id, JobStatus* job_status) {
  cout << "[RMC] finalizing job " << job_id << endl;
  MlServerContext::GlobalMutex.lock();
  MlServerContext::JobMetadataMap[job_id]->status = DONE;
  MlServerContext::JobStatusMap[job_id]->total_running_seconds +=
      Utils::get_time_second() -
      MlServerContext::JobStatusMap[job_id]->last_start_time;
  MlServerContext::GlobalMutex.unlock();

  // merge output files into one file
  string output_filename = Utils::gen_temp_filename();
  std::ofstream ofs(output_filename, std::ios_base::binary);

  vector<SubJob*> subjobs(job_status->finished_subjobs.begin(),
                          job_status->finished_subjobs.end());

  sort(subjobs.begin(), subjobs.end(),
       [](const SubJob* a, const SubJob* b) { return a->offset < b->offset; });

  for (auto subjob : subjobs) {
    string subjob_output_filename =
        FileSystemUtils::get_file_temp(subjob->output_filename);
    std::ifstream ifs(subjob_output_filename, std::ios_base::binary);
    ofs << ifs.rdbuf();
    ifs.close();
    remove(subjob_output_filename.c_str());
  }

  ofs.close();

  string sdfs_output_filename = job_id + "_output";
  FileSystemUtils::put_file(output_filename, sdfs_output_filename);
  remove(output_filename.c_str());

  // thread(
  //     [](vector<SubJob*> subjobs) {
  //       for (auto subjob : subjobs) {
  //         FileSystemUtils::delete_file(subjob->output_filename);
  //       }
  //     },
  //     subjobs)
  //     .detach();

  cout << "[RMC] finshed job " << job_id << ", the output file is stored as "
       << sdfs_output_filename << endl;
}

void MlUtils::assign_subjob_to_worker(Member worker, string job_id,
                                      SubJob* subjob) {
  MlServerContext::LocalMutex.lock();
  auto job = MlServerContext::JobMetadataMap[job_id];
  auto job_status = MlServerContext::JobStatusMap[job_id];

  subjob->set_start_time();
  job_status->running_subjobs[worker.to_member_id()] = subjob;
  job_status->running_worker_count += 1;
  MlServerContext::LocalMutex.unlock();

  cout << "[RMC] dispatching work..." << endl;
  MlRunSubjobRequest* request = new MlRunSubjobRequest(
      job->model, job->testset, job->label, subjob->offset, subjob->limit,
      subjob->output_filename);

  // TODO: timeout mechanism
  SocketMessage* response = nullptr;
  try {
    response = RequestSender::send_request(worker.hostname.c_str(),
                                           stoi(worker.port), request);
  } catch (exception& e) {
    cerr << "send request exception: " << e.what() << endl;
  }

  MlServerContext::LocalMutex.lock();
  if (response && response->message_type == SOCKET_MESSAGE_TYPE::RUN_SUBJOB) {
    cout << "[RMC] work done!" << endl;
    MlRunSubjobResponse* ml_response = (MlRunSubjobResponse*)response;

    subjob->set_end_time();
    job_status->finished_subjobs.insert(subjob);
    job_status->finished_query_count += ml_response->done_query_count;
    job_status->accurate_query_count += ml_response->accurate_query_count;
  } else {
    cout << "[RMC] work failed :( : " << worker.to_member_id() << endl;
    subjob->reset_time();
    job_status->todo_subjobs.push(subjob);
  }

  // update server status and job status
  job_status->running_subjobs.erase(worker.to_member_id());
  job_status->running_worker_count -= 1;
  MlServerContext::worker_pool.push(worker.to_member_id());

  MlServerContext::LocalMutex.unlock();

  if (job_status->running_subjobs.empty() && job_status->todo_subjobs.empty()) {
    finalize_job(job_id, job_status);
  }
}

double MlUtils::calculate_query_rate(JobMetadata* job_metadata,
                                     JobStatus* job_status,
                                     double current_timestamp,
                                     int recent_second /*=10*/) {
  int latest_subjob_count = 0;

  for (auto subjob : job_status->finished_subjobs) {
    if (current_timestamp - subjob->end_time <= recent_second) {
      latest_subjob_count += 1;
    }
  }
  return (recent_second > 0)
             ? ((double)latest_subjob_count * job_metadata->batch_size) /
                   recent_second
             : 0;
}

unordered_map<string, double> MlUtils::calculate_query_rates(
    unordered_map<string, JobMetadata*>& job_metadata_map,
    unordered_map<string, JobStatus*>& job_status_map) {
  unordered_map<string, double> query_rate_map;
  double current_timestamp = Utils::get_time_second();

  // algo 2: calculate most recent 10 seconds qps
  int recent_second = 30;
  for (auto const& [job_id, job_status] : job_status_map) {
    query_rate_map[job_id] = calculate_query_rate(
        job_metadata_map[job_id], job_status, current_timestamp, recent_second);
  }

  return query_rate_map;
}

unordered_map<string, double> MlUtils::calculate_average_query_rates(
    unordered_map<string, JobMetadata*>& job_metadata_map,
    unordered_map<string, JobStatus*>& job_status_map) {
  unordered_map<string, double> query_rate_map;
  double current_timestamp = Utils::get_time_second();

  // algo 1: calculate lifetime qps
  for (auto const& [job_id, job_status] : job_status_map) {
    double current_running_session = 0;

    if (job_metadata_map[job_id]->status != DONE)
      current_running_session = current_timestamp - job_status->last_start_time;

    double total_run_time =
        current_running_session + job_status->total_running_seconds;
    query_rate_map[job_id] = job_status->finished_query_count / total_run_time;
  }

  return query_rate_map;
}

/*
  distriubute_worker

  consider cases:
    case1: all query_rate are zero

    case2: some query_rate are zero
      case2-1 -> zero_query_rate job take worker first,
      case2-2 -> if query_rate_sum - query_rate == 0,
      case2-3 -> if query_rate_sum - query_rate != 0,

    case3: all query_rate are non-zero


  distribution rule:
  1. zero query rate case:
    -> takes (#worker / #job).floor

  2. non-zero query rate case (taking the rest of workers after distributed
  zero-query case):
    -> norm_query_rate = query_rate_sum - query_rate
    -> norm_sum = sum(norm_query_rates)
    -> takes (#worker * (norm_query_rate / norm_sum)).floor

  3. remaining workers are assign to the first job (least query rate job)
*/
unordered_map<string, int> MlUtils::distriubute_worker(
    unordered_map<string, JobMetadata*>& job_metadata_map,
    unordered_map<string, JobStatus*>& job_status_map,
    unordered_map<string, double> query_rate_map, int avaiable_worker) {
  unordered_map<string, int>
      distribution;  // job_id : number of worker for the job
  vector<tuple<double, string> > fresh_job_pool;  // jobs with query_rate == 0
  vector<tuple<double, string> > older_job_pool;  // jobs with query_rate > 0

  double query_rate_sum = 0.0;
  double norm_denominator = 0.0;

  for (auto const& [job_id, query_rate] : query_rate_map) {
    auto job_metadata = job_metadata_map[job_id];
    auto job_status = job_status_map[job_id];
    if (job_metadata->status == RUNNING &&
        job_status->todo_subjobs.size() > 0) {
      if (query_rate == 0) {
        fresh_job_pool.push_back(make_tuple(query_rate, job_id));
      } else {
        older_job_pool.push_back(make_tuple(query_rate, job_id));
        query_rate_sum += query_rate;
      }
    }
  }

  sort(older_job_pool.begin(), older_job_pool.end());

  // calculate norm_denominator after getting query_rate_sum
  for (auto tuple : older_job_pool) {
    double query_rate = get<0>(tuple);
    if (query_rate > 0) norm_denominator += query_rate_sum - query_rate;
  }

  int average_worker_number =
      avaiable_worker / (older_job_pool.size() + fresh_job_pool.size());
  int remaining_worker_number = avaiable_worker;

  // Step 1: distribute average # worker to jobs with zero query rate
  for (auto tuple : fresh_job_pool) {
    double query_rate = get<0>(tuple);
    string job_id = get<1>(tuple);
    distribution[job_id] = average_worker_number;
    remaining_worker_number -= distribution[job_id];
  }

  int avaiable_worker_number_for_older_jobs = remaining_worker_number;

  // Step 2: distribute workers for the jobs with query rate > 0
  if (norm_denominator > 0) {
    for (auto tuple : older_job_pool) {
      double query_rate = get<0>(tuple);
      string job_id = get<1>(tuple);
      double norm_query_rate = query_rate_sum - query_rate;
      distribution[job_id] = avaiable_worker_number_for_older_jobs *
                             (norm_query_rate / norm_denominator);
      remaining_worker_number -= distribution[job_id];
    }
  }

  // Step 3: distribute the remaining workers to older job in sorted order
  for (int i = 0; i < older_job_pool.size() && remaining_worker_number > 0;
       i = (i + 1) % older_job_pool.size()) {
    string job_id = get<1>(older_job_pool[i]);
    distribution[job_id] += 1;
    remaining_worker_number -= 1;
  }

  // Step 4: this round distribution = total distribution - # owned worker
  for (auto const& [job_id, _] : distribution) {
    distribution[job_id] = max(
        distribution[job_id] - job_status_map[job_id]->running_worker_count, 0);
  }

  return distribution;
}

int MlUtils::count_pending_job(
    unordered_map<string, JobMetadata*>& job_metadata_map,
    unordered_map<string, JobStatus*>& job_status_map) {
  int count = 0;

  for (auto const& [job_id, job_status] : job_status_map) {
    auto job_metadata = job_metadata_map[job_id];
    count +=
        (job_metadata->status == RUNNING && job_status->todo_subjobs.size() > 0)
            ? 1
            : 0;
  }

  return count;
}

void MlUtils::print_job_status(
    unordered_map<string, JobMetadata*>& job_metadata_map,
    unordered_map<string, JobStatus*>& job_status_map,
    unordered_map<string, double>& average_query_rate_map,
    unordered_map<string, double>& query_rate_map) {
  cout << "=========== [ML JOBS] ===========" << endl;

  for (auto const& [job_id, job_metadata] : job_metadata_map) {
    string progress = "N/A";
    int worker_num = 0;
    double accuracy = 0;
    double average_query_rate = 0;
    double query_rate = 0;
    if (job_status_map.count(job_id) != 0) {
      auto job_status = job_status_map[job_id];
      progress = to_string(job_status->finished_subjobs.size()) + " / " +
                 to_string(job_status->todo_subjobs.size() +
                           job_status->finished_subjobs.size() +
                           job_status->running_subjobs.size());

      worker_num = job_status->running_worker_count;

      accuracy = (double)job_status->accurate_query_count /
                 job_status->finished_query_count;
    }

    if (query_rate_map.count(job_id) != 0) {
      query_rate = query_rate_map[job_id];
    }

    if (average_query_rate_map.count(job_id) != 0) {
      average_query_rate = average_query_rate_map[job_id];
    }

    cout << "Job " << job_id << ": " << endl;
    cout << "\t Status: " << job_metadata->get_status() << endl;
    cout << "\t Progress: " << progress << endl;
    cout << "\t Worker: " << worker_num << endl;
    cout << "\t QPS: " << average_query_rate << endl;
    cout << "\t Latest QPS: " << query_rate << endl;
    cout << "\t Accuracy: " << accuracy << endl;
  }
}

void MlUtils::resource_management_cycle() {
  // only the leader of the group is allowed to run RMC
  if (MemberServerContext::SelfMember != MemberServerContext::Leader) return;

  cout << "[RMC] start RMC" << endl;
  MlServerContext::LocalMutex.lock();
  MlServerContext::GlobalMutex.lock();

  // worker pool remove failed member
  int worker_pool_size = MlServerContext::worker_pool.size();
  for (int i = 0; i < worker_pool_size; i++) {
    string worker_id = MlServerContext::worker_pool.front();
    MlServerContext::worker_pool.pop();
    if (MemberServerContext::RemovedMember.count(worker_id) == 0) {
      MlServerContext::worker_pool.push(worker_id);
    }
  }

  auto average_query_rate_map = calculate_average_query_rates(MlServerContext::JobMetadataMap,
                                              MlServerContext::JobStatusMap);
  auto query_rate_map = calculate_query_rates(MlServerContext::JobMetadataMap,
                                              MlServerContext::JobStatusMap);
  MlUtils::print_job_status(MlServerContext::JobMetadataMap,
                            MlServerContext::JobStatusMap, average_query_rate_map, query_rate_map);

  int job_count = count_pending_job(MlServerContext::JobMetadataMap,
                                    MlServerContext::JobStatusMap);

  cout << "[RMC] avaliable workers: " << MlServerContext::worker_pool.size()
       << endl;
  cout << "[RMC] pending jobs: " << job_count << endl;

  // check worker and job pools
  if (MlServerContext::worker_pool.size() == 0 || job_count == 0) {
    MlServerContext::LocalMutex.unlock();
    MlServerContext::GlobalMutex.unlock();
    cout << "[RMC] no worker/job available, exit RMC!" << endl;
    return;
  }

  // pair workers and sub-jobs fairly
  cout << "[RMC] paring workers and jobs!" << endl;
  auto job_worker_distribution = distriubute_worker(
      MlServerContext::JobMetadataMap, MlServerContext::JobStatusMap,
      query_rate_map, MlServerContext::all_workers.size());

  // dispatch subjobs to workers
  cout << "[RMC] start dispatching work!" << endl;
  for (auto const& [job_id, worker_number] : job_worker_distribution) {
    auto job_status = MlServerContext::JobStatusMap[job_id];

    // assign workers to a job's subjobs
    for (int i = 0; i < worker_number; i++) {
      if (job_status->todo_subjobs.empty() ||
          MlServerContext::worker_pool.empty())
        break;

      auto subjob = job_status->todo_subjobs.front();
      job_status->todo_subjobs.pop();

      auto worker_id = MlServerContext::worker_pool.front();
      MlServerContext::worker_pool.pop();
      Member worker = MemberUtils::parse_member_id(worker_id);

      thread(assign_subjob_to_worker, worker, job_id, subjob).detach();
    }
  }

  MlServerContext::LocalMutex.unlock();
  MlServerContext::GlobalMutex.unlock();
}

void MlUtils::update_ml_status(
    unordered_map<string, JobMetadata*>& job_metadata_map,
    unordered_map<string, JobStatus*>& job_status_map) {
  // leader does not need to update ml status
  if (MemberServerContext::SelfMember == MemberServerContext::Leader) return;

  MlServerContext::GlobalMutex.lock();
  MlServerContext::LocalMutex.lock();

  MlServerContext::JobMetadataMap = job_metadata_map;
  MlServerContext::JobStatusMap = job_status_map;

  MlServerContext::GlobalMutex.unlock();
  MlServerContext::LocalMutex.unlock();
}

void MlUtils::handle_running_subjobs() {
  MlServerContext::GlobalMutex.lock();
  MlServerContext::LocalMutex.lock();

  // place all running subjobs back to todo subjobs to ensure each subjob is
  // executed 'at least once'
  for (auto const& [job_id, job_status] : MlServerContext::JobStatusMap) {
    auto job_metadata = MlServerContext::JobMetadataMap[job_id];
    if (job_metadata->status == RUNNING &&
        job_status->running_subjobs.size() > 0) {
      for (auto const& [worker_id, subjob] : job_status->running_subjobs) {
        subjob->reset_time();
        job_status->todo_subjobs.push(subjob);
        MlServerContext::worker_pool.push(worker_id);
        if (MlServerContext::all_workers.count(worker_id) == 0) {
          MlServerContext::all_workers.insert(worker_id);
        }
      }
      job_status->running_subjobs.clear();
      job_status->running_worker_count = 0;
    }
  }

  MlServerContext::GlobalMutex.unlock();
  MlServerContext::LocalMutex.unlock();
}
