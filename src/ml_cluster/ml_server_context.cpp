#include "ml_server_context.h"

#include "../membership/member_server_context.h"
#include "../utils.h"

using namespace std;

// init static members
unordered_map<string, JobMetadata*> MlServerContext::JobMetadataMap;

unordered_map<string, JobStatus*> MlServerContext::JobStatusMap;

queue<string> MlServerContext::worker_pool;
set<string> MlServerContext::all_workers;

int MlServerContext::SequenceNumber = 0;

mutex MlServerContext::GlobalMutex;
mutex MlServerContext::LocalMutex;
mutex MlServerContext::InferenceResourceMutex;

void MlServerContext::CreateContext() {
  MlServerContext::GlobalMutex.unlock();
  MlServerContext::LocalMutex.unlock();
  MlServerContext::InferenceResourceMutex.unlock();

  MlServerContext::GlobalMutex.lock();
  MlServerContext::LocalMutex.lock();

  // reset job metadata
  MlServerContext::JobMetadataMap = unordered_map<string, JobMetadata*>();

  MlServerContext::JobStatusMap = unordered_map<string, JobStatus*>();

  MlServerContext::worker_pool = queue<string>();
  MlServerContext::all_workers = set<string>();
  worker_pool.push(MemberServerContext::SelfMember);
  all_workers.insert(MemberServerContext::SelfMember);

  MlServerContext::SequenceNumber = 0;

  MlServerContext::LocalMutex.unlock();
  MlServerContext::GlobalMutex.unlock();
}

void MlServerContext::ReleaseContext() {
  // TODO: release all context here
}

JobStatus::JobStatus() {
  this->last_start_time = Utils::get_time_second();
  this->total_running_seconds = 0;
  this->finished_query_count = 0;
  this->accurate_query_count = 0;
  this->running_worker_count = 0;
  this->running_subjobs = unordered_map<string, SubJob*>();
  this->todo_subjobs = queue<SubJob*>();
  this->finished_subjobs = set<SubJob*>();
}

void JobStatus::start_job(JobStatus* jobStatus) {
  jobStatus->last_start_time = Utils::get_time_second();
}

void JobStatus::stop_job(JobStatus* jobStatus) {
  // clear running worker count and running subjobs
  jobStatus->running_worker_count = 0;
  jobStatus->running_subjobs = unordered_map<string, SubJob*>();
  // save running time
  jobStatus->total_running_seconds +=
      Utils::get_time_second() - jobStatus->last_start_time;
}
