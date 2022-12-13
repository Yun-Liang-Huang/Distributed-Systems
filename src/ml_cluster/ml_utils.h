#ifndef ML_UTILS
#define ML_UTILS

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

class MlUtils {
 public:
  static string get_ml_output_filename(string job, int number);

  static JobStatus* create_job_status(string job, JobMetadata* jobMetadata);

  static void update_ml_worker_pool();

  static void reset_ml_worker_pool();

  static void finalize_job(string job_id, JobStatus* job_status);

  static void assign_subjob_to_worker(Member worker, string job_id,
                                      SubJob* subjob);

  static double calculate_query_rate(JobMetadata* job_metadata,
                                     JobStatus* job_status,
                                     double current_timestamp,
                                     int recent_second = 10);

  static unordered_map<string, double> calculate_average_query_rates(
      unordered_map<string, JobMetadata*>& job_metadata_map,
      unordered_map<string, JobStatus*>& job_status_map);

  static unordered_map<string, double> calculate_query_rates(
      unordered_map<string, JobMetadata*>& job_metadata_map,
      unordered_map<string, JobStatus*>& job_status_map);

  static unordered_map<string, int> distriubute_worker(
      unordered_map<string, JobMetadata*>& job_metadata_map,
      unordered_map<string, JobStatus*>& job_status_map,
      unordered_map<string, double> query_rate_map, int avaiable_worker);

  static int count_pending_job(
      unordered_map<string, JobMetadata*>& job_metadata_map,
      unordered_map<string, JobStatus*>& job_status_map);

  static void print_job_status(
      unordered_map<string, JobMetadata*>& job_metadata_map,
      unordered_map<string, JobStatus*>& job_status_map,
      unordered_map<string, double>& query_rate_map,
      unordered_map<string, double>& latest_query_rate_map);

  static void resource_management_cycle();

  static void update_ml_status(
      unordered_map<string, JobMetadata*>& job_metadata_map,
      unordered_map<string, JobStatus*>& job_status_map);

  static void handle_running_subjobs();
};

#endif