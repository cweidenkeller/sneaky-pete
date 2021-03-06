#ifndef __NOVA_GUEST_BACKUP_BACKUPRESTORERUNNER_H
#define __NOVA_GUEST_BACKUP_BACKUPRESTORERUNNER_H

#include "nova/process.h"
#include "nova/utils/regex.h"
#include <boost/shared_ptr.hpp>
#include <string>


namespace nova { namespace guest { namespace backup {

    class BackupRestoreInfo {
        public:
            BackupRestoreInfo(const std::string & token,
                          const std::string & backup_url,
                          const std::string & backup_checksum);

            const std::string & get_backup_url() const {
                return backup_url;
            }

            const std::string & get_token() const {
                return token;
            }

            const std::string & get_backup_checksum() const {
                return backup_checksum;
            }

        private:
            std::string token;
            std::string backup_url;
            std::string backup_checksum;
    };


    class BackupRestoreManager {
        public:
            BackupRestoreManager(const nova::process::CommandList command_list,
                                 const std::string & delete_file_pattern,
                                 const std::string & restore_directory,
                                 const std::string & save_file_pattern,
                                 const size_t zlib_buffer_size);

            void run(const BackupRestoreInfo & restore);

        private:
            class BackupRestoreJob;

            const nova::process::CommandList commands;
            const nova::utils::Regex delete_file_pattern;
            const std::string restore_directory;
            const nova::utils::Regex save_file_pattern;
            const size_t zlib_buffer_size;
    };


    typedef boost::shared_ptr<BackupRestoreManager> BackupRestoreManagerPtr;


    class BackupRestoreException  : public std::exception {
        public:
            virtual const char * what() const throw();
    };

} } }  // end namespace

#endif //__NOVA_GUEST_BACKUP_BACKUPRESTORERUNNER_H
