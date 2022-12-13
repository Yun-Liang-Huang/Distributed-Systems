#include "ml_api_handler.h"

#include "../client_request_sender.h"
#include "../membership/member_server_context.h"
#include "../membership/member_utils.h"
#include "../socket_message.h"
#include "../socket_message_utils.h"
#include "../utils.h"
#include "ml_add_job_request.h"
#include "ml_remove_job_request.h"
#include "ml_run_subjob_request.h"
#include "ml_server_context.h"
#include "ml_start_job_request.h"
#include "ml_status_request.h"
#include "ml_status_response.h"
#include "ml_stop_job_request.h"
#include "ml_utils.h"
#include "ml_worker_utils.h"

using namespace std;

SocketMessage* create_job_metadata(string job, string model, string testset,
                                   string label, int batch_size) {
  SocketMessage* response;
  MlServerContext::GlobalMutex.lock();
  // job name cannot be duplicated
  if (MlServerContext::JobMetadataMap.count(job) == 0) {
    MlServerContext::JobMetadataMap[job] =
        new JobMetadata(IDLE, model, testset, label, batch_size);
    response = new SuccessfulResponse();
  } else {
    cout << Utils::getTime()
         << "create job failed, duplicated job name: " << job << endl;
    response = new FailedResponse();
  }

  int seq_number = MlServerContext::SequenceNumber++;
  MlServerContext::GlobalMutex.unlock();
  return response;
}

SocketMessage* remove_job_metadata(string job) {
  SocketMessage* response;
  MlServerContext::GlobalMutex.lock();
  if (MlServerContext::JobMetadataMap.count(job) != 0) {
    MlServerContext::JobMetadataMap.erase(job);
    response = new SuccessfulResponse();
  } else {
    cout << Utils::getTime() << "remove job failed, job: " << job
         << " does not exist" << endl;
    response = new FailedResponse();
  }

  int seq_number = MlServerContext::SequenceNumber++;
  MlServerContext::GlobalMutex.unlock();
  return response;
}

SocketMessage* coordinator_start_job(string job) {
  SocketMessage* response;
  MlServerContext::GlobalMutex.lock();
  MlServerContext::LocalMutex.lock();
  if (MlServerContext::JobMetadataMap.count(job) != 0) {
    // if job status exist and status is IDLE: restart job
    // else: create new job status

    if (MlServerContext::JobMetadataMap[job]->status == IDLE) {
      MlServerContext::JobMetadataMap[job]->status = RUNNING;
      if (MlServerContext::JobStatusMap.count(job) != 0) {
        JobStatus::start_job(MlServerContext::JobStatusMap[job]);
      } else {
        MlServerContext::JobStatusMap[job] = MlUtils::create_job_status(
            job, MlServerContext::JobMetadataMap[job]);
      }
    } else {
      cout << Utils::getTime() << "job: " << job << " cannot be started"
           << endl;
    }
    response = new SuccessfulResponse();
  } else {
    cout << Utils::getTime() << "start job failed, job: " << job
         << " does not exist" << endl;
    response = new FailedResponse();
  }

  int seq_number = MlServerContext::SequenceNumber++;
  MlServerContext::GlobalMutex.unlock();
  MlServerContext::LocalMutex.unlock();
  return response;
}

SocketMessage* coordinator_stop_job(string job) {
  SocketMessage* response;
  MlServerContext::GlobalMutex.lock();
  MlServerContext::LocalMutex.lock();
  if (MlServerContext::JobStatusMap.count(job) != 0) {
    if (MlServerContext::JobMetadataMap[job]->status == RUNNING) {
      MlServerContext::JobMetadataMap[job]->status == IDLE;
      JobStatus::stop_job(MlServerContext::JobStatusMap[job]);
    } else {
      cout << Utils::getTime() << "job: " << job << " cannot be stopped"
           << endl;
    }
    response = new SuccessfulResponse();
  } else {
    cout << Utils::getTime() << "stop job failed, job: " << job
         << " does not exist" << endl;
    response = new FailedResponse();
  }

  int seq_number = MlServerContext::SequenceNumber++;
  MlServerContext::GlobalMutex.unlock();
  MlServerContext::LocalMutex.unlock();
  return response;
}

SocketMessage* forward_request(SocketMessage* request, Member member) {
  SocketMessage* response = nullptr;
  try {
    response = RequestSender::send_request(member.hostname.c_str(),
                                           stoi(member.port), request);

    if (!response) {
      cout << Utils::getTime()
           << "Cannot forward request to member: " << member.hostname << ":"
           << member.port << endl;
    }
  } catch (exception e) {
    cerr << e.what() << endl;
  }
  return response;
}

SocketMessage* add_job(string job, string model, string testset, string label,
                       int batch_size) {
  // Coordinator: create job metadata
  // Not coordinator: forward add job request to the coordinator
  if (MemberServerContext::Leader == MemberServerContext::SelfMember) {
    return create_job_metadata(job, model, testset, label, batch_size);
  } else {
    Member leader = MemberUtils::parse_member_id(MemberServerContext::Leader);
    MlAddJobRequest* request =
        new MlAddJobRequest(job, model, testset, label, batch_size);
    return forward_request(request, leader);
  }
}

SocketMessage* remove_job(string job) {
  // Coordinator: remove job
  // Not coordinator: forward remove job request to the coordinator
  if (MemberServerContext::Leader == MemberServerContext::SelfMember) {
    return remove_job_metadata(job);
  } else {
    Member leader = MemberUtils::parse_member_id(MemberServerContext::Leader);
    MlRemoveJobRequest* request = new MlRemoveJobRequest(job);
    return forward_request(request, leader);
  }
}

SocketMessage* start_job(string job) {
  // Coordinator: start job
  // Not coordinator: forward start job request to the coordinator
  if (MemberServerContext::Leader == MemberServerContext::SelfMember) {
    return coordinator_start_job(job);
  } else {
    Member leader = MemberUtils::parse_member_id(MemberServerContext::Leader);
    MlStartJobRequest* request = new MlStartJobRequest(job);
    return forward_request(request, leader);
  }
}

SocketMessage* stop_job(string job) {
  // Coordinator: stop job
  // Not coordinator: forward stop job request to the coordinator
  if (MemberServerContext::Leader == MemberServerContext::SelfMember) {
    return coordinator_stop_job(job);
  } else {
    Member leader = MemberUtils::parse_member_id(MemberServerContext::Leader);
    MlStopJobRequest* request = new MlStopJobRequest(job);
    return forward_request(request, leader);
  }
}

SocketMessage* coordinator_calculate_job_status(MlStatusRequest* request) {
  MlStatusResponse* response = new MlStatusResponse();
  string job_id = request->job_id;
  auto job_metadata = MlServerContext::JobMetadataMap[job_id];
  auto job_status = MlServerContext::JobStatusMap[job_id];
  if (!job_metadata || !job_status) return new FailedResponse();

  // lastest 10 sec query rate
  double current_time = Utils::get_time_second();
  auto query_rate_map = MlUtils::calculate_average_query_rates(MlServerContext::JobMetadataMap, MlServerContext::JobStatusMap);
  response->query_rate = query_rate_map[job_id];
  response->latest_query_rate = MlUtils::calculate_query_rate(
      job_metadata, job_status, current_time, 30);

  // query count
  response->processed_query_count = job_status->finished_query_count;

  // process time data
  vector<SubJob*> finished_subjobs(job_status->finished_subjobs.begin(),
                                   job_status->finished_subjobs.end());

  if (finished_subjobs.size() > 0) {
    double process_time_sum = 0;
    for (auto subjob : finished_subjobs) {
      process_time_sum += subjob->get_processed_time();
    }

    // average
    double batch_average = (double)process_time_sum / (finished_subjobs.size());
    response->average_query_processing_time =
        batch_average / job_metadata->batch_size;

    // standard deviation
    if (finished_subjobs.size() > 1) {
      double deviation = 0;

      for (auto subjob : finished_subjobs)
        deviation += pow((subjob->get_processed_time() - batch_average) / job_metadata->batch_size, 2);

      response->std_deviation_query_processing_time =
          sqrt(deviation / finished_subjobs.size());
    }

    // sort subjobs by processing time ascendingly
    sort(finished_subjobs.begin(), finished_subjobs.end(),
         [](SubJob* a, SubJob* b) {
           return a->get_processed_time() > b->get_processed_time();
         });

    response->median_query_processing_time =
        finished_subjobs[floor(finished_subjobs.size() * 0.5)]
            ->get_processed_time() /
        job_metadata->batch_size;

    response->percentile_90_query_processing_time =
        finished_subjobs[floor(finished_subjobs.size() * 0.9)]
            ->get_processed_time() /
        job_metadata->batch_size;

    response->percentile_95_query_processing_time =
        finished_subjobs[floor(finished_subjobs.size() * 0.95)]
            ->get_processed_time() /
        job_metadata->batch_size;

    response->percentile_99_query_processing_time =
        finished_subjobs[floor(finished_subjobs.size() * 0.99)]
            ->get_processed_time() /
        job_metadata->batch_size;
  }

  // assigned worker
  for (auto const& [worker_id, _] : job_status->running_subjobs)
    response->assigned_workers.push_back(worker_id);

  return response;
}

SocketMessage* calculate_job_status(MlStatusRequest* request) {
  // Coordinator: calculate job status
  // Not coordinator: forward request to the coordinator
  if (MemberServerContext::Leader == MemberServerContext::SelfMember) {
    return coordinator_calculate_job_status(request);
  } else {
    return forward_request(request, Member(MemberServerContext::Leader));
  }
}

SocketMessage* MlApiHandler::api_handler(const char* buffer) {
  SocketMessage* output = nullptr;
  vector<string> tokens = SocketMessageUtils::parse_tokens(buffer);

  try {
    SOCKET_MESSAGE_TYPE message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
    if (ADD_JOB == message_type) {
      cout << Utils::getTime() << "ADD_JOB args: " << tokens[1] << " "
           << tokens[2] << " " << tokens[3] << " " << tokens[4] << " "
           << tokens[5] << endl;
      output =
          add_job(tokens[1], tokens[2], tokens[3], tokens[4], stoi(tokens[5]));

    } else if (REMOVE_JOB == message_type) {
      cout << Utils::getTime() << "REMOVE_JOB args: " << tokens[1] << endl;
      output = remove_job(tokens[1]);

    } else if (START_JOB == message_type) {
      cout << Utils::getTime() << "START_JOB args: " << tokens[1] << endl;
      output = start_job(tokens[1]);

    } else if (STOP_JOB == message_type) {
      cout << Utils::getTime() << "STOP_JOB args: " << tokens[1] << endl;
      output = stop_job(tokens[1]);

    } else if (START_PHASE == message_type) {
      cout << Utils::getTime() << "START_PHASE args: " << tokens[1] << endl;

    } else if (STOP_PHASE == message_type) {
      cout << Utils::getTime() << "STOP_PHASE args: " << tokens[1] << endl;

    } else if (RUN_SUBJOB == message_type) {
      cout << Utils::getTime() << "RUN_SUBJOB args: " << tokens[1] << endl;
      MlRunSubjobRequest* request = new MlRunSubjobRequest(buffer);
      MlServerContext::InferenceResourceMutex.lock();
      output = MlWorkerUtils::run_subjob_handler(request);
      MlServerContext::InferenceResourceMutex.unlock();
      delete request;
    } else if (ML_JOB_STATUS == message_type) {
      cout << Utils::getTime() << "ML_JOB_STATUS args: " << tokens[1] << endl;
      MlStatusRequest* request = new MlStatusRequest(buffer);
      output = calculate_job_status(request);
      delete request;
    } else {
      cout << Utils::getTime() << "Request not found" << endl;
    }
  } catch (invalid_argument& e) {
    cout << Utils::getTime() << "Failed to parse socket request: " << e.what()
         << endl;
    return new FailedResponse();
  }

  if (output == nullptr) {
    output = new SuccessfulResponse();
  }

  cout << Utils::getTime() << "Result: " << output << endl;
  return output;
}
