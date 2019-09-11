#include <zip.h>
#include <string>
#include <iostream>
#include <stdexcept>

class zipfile{
    public:
    zipfile(const char* filename="data.zip"){
        int errorp;
        zipper = zip_open(filename, ZIP_CREATE, &errorp);
        if(zipper == nullptr){
            zip_error_t ziperror;
            zip_error_init_with_code(&ziperror, errorp);
            throw std::runtime_error("Failed to open output file " + std::string(filename) + ": " + zip_error_strerror(&ziperror));
        }
        //add_dir("posts/");
        //add_dir("assets/");
    };
    ~zipfile(){
        if(zip_close(zipper) < 0){
            zip_discard(zipper);
            throw std::runtime_error("Failed to save zip file " + std::string(get_error()));
        }
    };
    inline bool add_dir(const char* path){
        if(has_file(path)){
            return true;
        }
        if (zip_dir_add(zipper, path, ZIP_FL_ENC_UTF_8) < 0) {
            return false;
        }
        return true;
    }
    inline bool add_file(std::string data, const char* path){
        zip_source_t *source  = zip_source_buffer(zipper, data.c_str(), data.size(), 0);
        if(source == nullptr){
            return false;
        }
        if(zip_file_add(zipper, path, source, ZIP_FL_ENC_UTF_8) < 0){
            zip_source_free(source);
            return false;
        }
        return true;
    }
    inline const char* get_error(){
        return zip_strerror(zipper);
    }
    inline bool has_file(const char* path){
        return zip_name_locate(zipper, path, 0u) >= 0;
    }
    private:
    zip_t *zipper;
};