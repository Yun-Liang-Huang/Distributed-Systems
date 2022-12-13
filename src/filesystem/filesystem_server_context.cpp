#include "filesystem_server_context.h"

#include "../utils.h"

using namespace std;

// -------------- generating development data START -------------- //

// -------------- generating development data END -------------- //

// init static members
unordered_map<string, FileMetadata*> FileSystemServerContext::FileMetadataMap;

unordered_map<string, FileVersionMetadata*> FileSystemServerContext::FileMap;

unordered_set<string> FileSystemServerContext::RecoveredFailureServers;

int FileSystemServerContext::SequenceNumber = 1;
mutex FileSystemServerContext::GlobalMutex;
mutex FileSystemServerContext::LocalMutex;

void FileSystemServerContext::CreateContext() {
  FileSystemServerContext::GlobalMutex.unlock();
  FileSystemServerContext::LocalMutex.unlock();

  FileSystemServerContext::GlobalMutex.lock();
  FileSystemServerContext::LocalMutex.lock();

  // reset file metadata
  FileSystemServerContext::FileMetadataMap =
      unordered_map<string, FileMetadata*>();

  FileSystemServerContext::FileMap =
      unordered_map<string, FileVersionMetadata*>();

  FileSystemServerContext::RecoveredFailureServers = unordered_set<string>();

  FileSystemServerContext::SequenceNumber = 1;

  FileSystemServerContext::LocalMutex.unlock();
  FileSystemServerContext::GlobalMutex.unlock();
}

void FileSystemServerContext::ReleaseContext() {
  // TODO: release all context here
}
