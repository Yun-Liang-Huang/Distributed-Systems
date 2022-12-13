#ifndef UTILS
#define UTILS

#include <math.h>
#include <stdlib.h> /* srand, rand */
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <array>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "query/query_log_response.h"
#include "socket_message_const.h"

#define HOST_NAME_MAX 1024

using namespace std;

class Utils {
 public:
  // credit: https://stackoverflow.com/a/478960
  static string exec(const char* cmd) {
    array<char, 1024> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
      throw runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
      result += buffer.data();
    }
    return result;
  }

  static string getHostName() {
    char hostname[HOST_NAME_MAX];
    int result = gethostname(hostname, HOST_NAME_MAX);
    if (result) {
      perror("gethostname");
      exit(0);
    }
    return (string)hostname;
  }

  // credit: https://stackoverflow.com/a/30425945
  static void start(int interval_ms, std::function<void(void)> func) {
    std::thread([=]() {
      while (true) {
        func();
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
      }
    }).detach();
  }

  static string getTime() {
    char buffer[26];
    int millisec;
    struct tm* tm_info;
    struct timeval tv;

    gettimeofday(&tv, NULL);

    millisec = lrint(tv.tv_usec / 1000.0);  // Round to nearest millisec
    if (millisec >= 1000) {  // Allow for rounding up to nearest second
      millisec -= 1000;
      tv.tv_sec++;
    }

    tm_info = localtime(&tv.tv_sec);

    strftime(buffer, 26, "%Y:%m:%d %H:%M:%S", tm_info);

    return "[" + (string)buffer + "." + to_string(millisec) + "] ";
  }

  static bool probability(int pass, int base) { return (pass > rand() % base); }

  // credit:
  // https://www.techiedelight.com/implode-a-vector-of-strings-into-a-comma-separated-string-in-cpp/
  static string join(vector<string> const& strings, string delim) {
    stringstream ss;
    copy(strings.begin(), strings.end(),
         ostream_iterator<string>(ss, delim.c_str()));
    return ss.str();
  }

  /**
   *
   * credit:
   * https://unix.stackexchange.com/questions/138398/how-to-get-lines-10-to-100-from-a-200-line-file-into-a-new-file
   *
   * e.g., sed -n -e '10,100p' input.txt > output.txt
   */
  static void crop_file(string filename, string output_filename, int from_line,
                        int to_line) {
    string command = "sed -n -e '" + to_string(from_line) + "," +
                     to_string(to_line) + "p' " + filename + " > " +
                     output_filename;
    exec(command.c_str());
  }

  /**
   * credit:
   * https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exists-using-standard-c-c11-14-17-c
   */
  static bool is_file_exists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
  }

  static string gen_temp_filename() {
    char temp_local_filename[40];
    tmpnam(temp_local_filename);

    return temp_local_filename;
  }

  static double get_time_second() {
    return (double)chrono::system_clock::now().time_since_epoch().count() / 1e9;
  }

  static int count_file_line(string filename) {
    std::ifstream ifs(filename);
    int line_count = std::count(std::istreambuf_iterator<char>(ifs),
                                std::istreambuf_iterator<char>(), '\n');
    return line_count;
  }
};

#endif