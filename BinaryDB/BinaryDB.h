#pragma once
#include <fstream>
#include <BinaryList.h>
#include <platform/macros.h>
#include <vector>
#include <Loader/Commpress/Commpress.h>

#define FILE_CLASTER_SIZE 8192 //8 кб

class BinaryDB;

struct FileClaster
{
	//Указатель на следующий блок
	FileClaster* pNext = nullptr;

	//Представление данных в памяти
	char* pData = nullptr;

	size_t iLocalId  = -1;

	~FileClaster();
};
 
class DataDecriptor
{
	friend class BinaryDB;
public:

	//Возвращет размер в байтах
	size_t getSize() const;
	 
	const std::string& getName() const;

private:
	//Указатель на владельца
	const BinaryDB* _pOwner = nullptr;

	//Указатель на первый кластер 
	FileClaster* _pFirstNode = nullptr;

	//Размер несжатых данных
	size_t _nBytes = -1;

	//Размер сжатых данных
	size_t _nCompressBytes = -1;
	
	//Локальное имя файла
	std::string _sName; 
};

class DataFileObject
{
	friend class BinaryDB;
public:

	DataFileObject(const BinaryDB* pOwner, const std::string& name);
	 
	//Описание записи
	CL::BinaryTableRecord objectFild;  
	 
	const std::string& getName() const;
	  
	const BinaryDB* getOwner() const;

	const DataDecriptor* getFile(const std::string& file) const;

	~DataFileObject();

	//Вернет true, если объект был изменен, иначе false
	bool onChange() const;

private:
	bool _bOnChange = false;

	//Имя записи
	std::string _sName;

	//Локальные данные
	std::map<std::string, DataDecriptor*> _localFileMap;

	//Узатель на владельца
	const BinaryDB* _pOwner;
	 
	DataFileObject(const DataFileObject&) = delete;
	DataFileObject operator = (const DataFileObject&) = delete; 
};
 
struct ClasterIterator
{
	FileClaster* pCurrentClaster = nullptr;

	size_t iPosition = 0;
	size_t iClasterCount = 0;
	bool bReadFromMem = false;
	char* pData = nullptr;

	bool iterate(); 
};

class BinaryDB
{
public:
	 
	//Конструктор
	//sFolder - папка с файлом 
	//sFileName - имя
	//bOverride - если true, очистит файл
	BinaryDB(const std::string& sFolder, const std::string& sFileName, bool bOverride = false);
	 
	//Если из объекта данных будет удален файл, вернет true, иначе false
	//pDataObject - указатель на объект
	//sFileName   - имя файла
	bool erase(DataFileObject* pDataObject, const std::string& sFileName);

	//Удалит объект данных c именем sName
	//Вернет true если объект удален, иначе false
	bool erase(const std::string& sName);

	//Переименутет объект с именем sOldName в sNewName
	//Вернет true, если объект был успешно переименован, иначе false
	bool rename(const std::string& sOldName, const std::string& sNewName);

	//Вернет true, если объект с именем sName - существует, иначе false
	bool exist(const std::string& sName) const;

	//Записывает данные 
	//pDataObject - указатель на объект
	//sFileName - имя файла
	//pData - данные
	//nDataSize - размер данных
	//Вернет true - если данные записаны успешно, вернет false если:
	//- pDataObject == null или не принадлжет к этой базе данных
	//- sFileName нулевая строка или файл с таким именем уже есть в pDataObject
	//- nDataSize = 0 
	bool write(DataFileObject* pDataObject, const std::string& sFileName, char* pData, size_t nDataSize);

	//Читает данные файла
	//pDescriptor - дескриптор файла
	//pData - буфер
	//Вернет true если чтение успешное, false если pDescriptor не от этой базы данных
	bool read(const DataDecriptor* pDescriptor, char* pData);
	 
	//Вернет указатель на созданный объект данных. Если он уже существует, вернет nullptr
	DataFileObject* createFileManifest(const std::string& sName);

	//Вернет указатель на объект данных, если он уже существует, иначе он вернет nullptr
	DataFileObject* findFileManifest(const std::string& sName);
  
	~BinaryDB();

	//Сохраняет изменения, если они есть
	void save();

	size_t getClasterCount() const;

	//Вернет true, если объект был изменен, иначе false
	bool onChange() const;

private: 
	bool _bOnChange = false;

	std::string _sName;
	std::string _manifestFilePatch;
	std::string _dataFilePatch;
	size_t _nAlocatedClasters = 0;

	std::fstream _manifestFile, _dataFile;
	std::map<std::string, DataFileObject*> _manifestMap;
	std::vector<FileClaster*> _mClaster;

	FileClaster* pFirstFreeClaster = nullptr;
	 
	BinaryDB(const BinaryDB&) = delete;
	BinaryDB& operator = (const BinaryDB&) = delete;
	 
	size_t getFreeClasterCount() const;

	void alocate(size_t nBlock);

	void readManifest();


	
};