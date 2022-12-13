#ifndef ML_WORKER_UTILS
#define ML_WORKER_UTILS

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

class MlWorkerUtils {
 public:
  static int calculate_accurate_count(string output_filename,
                                      string label_filename, int offset,
                                      int limit) {
    std::ifstream label(label_filename);
    std::ifstream output(output_filename);
    int match_count = 0;

    // compare output and label line by line
    for (string output_line, label_line;
         getline(output, output_line), getline(label, label_line);) {
      if (output_line == label_line) match_count += 1;
    }

    return match_count;
  }

  static SocketMessage* run_subjob_handler(MlRunSubjobRequest* request) {
    // read model and testcase from sdfs
    string temp_model_filename = FileSystemUtils::get_file_temp(request->model);
    string temp_testcase_filename =
        FileSystemUtils::get_file_temp(request->testcase);

    string temp_label_filename = FileSystemUtils::get_file_temp(request->label);
    string temp_output_filename = Utils::gen_temp_filename();

    // crop testcase, only keep the lines that are runing in this batch
    string cropped_testcase_filename = temp_testcase_filename + ".cropped";
    Utils::crop_file(temp_testcase_filename, cropped_testcase_filename,
                     request->offset + 1, request->offset + request->limit);
    string cropped_label_filename = temp_label_filename + ".cropped";
    Utils::crop_file(temp_label_filename, cropped_label_filename,
                     request->offset + 1, request->offset + request->limit);

    // start running python script
    execute_python_script(temp_model_filename, cropped_testcase_filename,
                          temp_output_filename);

    // write output to sdfs
    FileSystemUtils::put_file(temp_output_filename, request->output_filename);

    // put real accurate number
    double accurate_count =
        calculate_accurate_count(temp_output_filename, cropped_label_filename,
                                 request->offset, request->limit);

    // delete local temp files
    std::remove(temp_output_filename.c_str());
    std::remove(temp_model_filename.c_str());
    std::remove(temp_testcase_filename.c_str());
    std::remove(cropped_testcase_filename.c_str());
    std::remove(temp_label_filename.c_str());
    std::remove(cropped_label_filename.c_str());

    MlRunSubjobResponse* response = new MlRunSubjobResponse(
        request->output_filename, request->limit, accurate_count);
    return response;
  }

  static void execute_python_script(string script_filename,
                                    string input_filename,
                                    string output_filename) {
    string command = "python3 " + script_filename + " " + input_filename;

    FILE* result_fp = popen(command.c_str(), "r");

    if (result_fp == NULL)
      throw std::runtime_error("Cannot read output from python script");

    // Create and open a text file
    vector<string> outputs;

    char buffer[SCRIPT_RESULT_BUFFER_SIZE] = {0};
    while (fgets(buffer, SCRIPT_RESULT_BUFFER_SIZE, result_fp)) {
      // cout << buffer;
      outputs.push_back(string(buffer));
    }

    if (outputs.size() != Utils::count_file_line(input_filename)) {
      throw std::runtime_error(
          "inference execution error: input and output line doesn't match");
    }

    ofstream output_stream(output_filename);
    for (auto output : outputs) output_stream << string(output);
    output_stream.close();
  }
};

#endif