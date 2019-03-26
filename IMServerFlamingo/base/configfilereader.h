/** 
 *  简单的配置文件读取类，ConfigFileReader.h
 */
#ifndef BASE_CONFIG_FILE_READER_H_
#define BASE_CONFIG_FILE_READER_H_
#include <map>
#include <string>

class CConfigFileReader
{
public:
	CConfigFileReader(const char* filename);
	~CConfigFileReader();

    char* GetConfigName(const char* name);
    int SetConfigValue(const char* name, const char*  value);

private:
    void  _LoadFile(const char* filename);
    int   _WriteFIle(const char*filename = NULL);
    void  _ParseLine(char* line);
    char* _TrimSpace(char* name);

    bool                                m_load_ok;
    std::map<std::string, std::string>  m_config_map;
    std::string                         m_config_file;
};


#endif //BASE_CONFIG_FILE_READER_H_
