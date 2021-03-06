#pragma once
#include <fstream>
#include <BinaryList.h>
#include <platform/macros.h>
#include <vector>
#include <Loader/Commpress/Commpress.h>

#define FILE_CLASTER_SIZE 8192 //8 ��

class BinaryDB;

struct FileClaster
{
	//��������� �� ��������� ����
	FileClaster* pNext = nullptr;

	//������������� ������ � ������
	char* pData = nullptr;

	size_t iLocalId  = -1;

	~FileClaster();
};
 
class DataDecriptor
{
	friend class BinaryDB;
public:

	//��������� ������ � ������
	size_t getSize() const;
	 
	const std::string& getName() const;

private:
	//��������� �� ���������
	const BinaryDB* _pOwner = nullptr;

	//��������� �� ������ ������� 
	FileClaster* _pFirstNode = nullptr;

	//������ �������� ������
	size_t _nBytes = -1;

	//������ ������ ������
	size_t _nCompressBytes = -1;
	
	//��������� ��� �����
	std::string _sName; 
};

class DataFileObject
{
	friend class BinaryDB;
public:

	DataFileObject(const BinaryDB* pOwner, const std::string& name);
	 
	//�������� ������
	CL::BinaryTableRecord objectFild;  
	 
	const std::string& getName() const;
	  
	const BinaryDB* getOwner() const;

	const DataDecriptor* getFile(const std::string& file) const;

	~DataFileObject();

	//������ true, ���� ������ ��� �������, ����� false
	bool onChange() const;

private:
	bool _bOnChange = false;

	//��� ������
	std::string _sName;

	//��������� ������
	std::map<std::string, DataDecriptor*> _localFileMap;

	//������� �� ���������
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
	 
	//�����������
	//sFolder - ����� � ������ 
	//sFileName - ���
	//bOverride - ���� true, ������� ����
	BinaryDB(const std::string& sFolder, const std::string& sFileName, bool bOverride = false);
	 
	//���� �� ������� ������ ����� ������ ����, ������ true, ����� false
	//pDataObject - ��������� �� ������
	//sFileName   - ��� �����
	bool erase(DataFileObject* pDataObject, const std::string& sFileName);

	//������ ������ ������ c ������ sName
	//������ true ���� ������ ������, ����� false
	bool erase(const std::string& sName);

	//������������ ������ � ������ sOldName � sNewName
	//������ true, ���� ������ ��� ������� ������������, ����� false
	bool rename(const std::string& sOldName, const std::string& sNewName);

	//������ true, ���� ������ � ������ sName - ����������, ����� false
	bool exist(const std::string& sName) const;

	//���������� ������ 
	//pDataObject - ��������� �� ������
	//sFileName - ��� �����
	//pData - ������
	//nDataSize - ������ ������
	//������ true - ���� ������ �������� �������, ������ false ����:
	//- pDataObject == null ��� �� ���������� � ���� ���� ������
	//- sFileName ������� ������ ��� ���� � ����� ������ ��� ���� � pDataObject
	//- nDataSize = 0 
	bool write(DataFileObject* pDataObject, const std::string& sFileName, char* pData, size_t nDataSize);

	//������ ������ �����
	//pDescriptor - ���������� �����
	//pData - �����
	//������ true ���� ������ ��������, false ���� pDescriptor �� �� ���� ���� ������
	bool read(const DataDecriptor* pDescriptor, char* pData);
	 
	//������ ��������� �� ��������� ������ ������. ���� �� ��� ����������, ������ nullptr
	DataFileObject* createFileManifest(const std::string& sName);

	//������ ��������� �� ������ ������, ���� �� ��� ����������, ����� �� ������ nullptr
	DataFileObject* findFileManifest(const std::string& sName);
  
	~BinaryDB();

	//��������� ���������, ���� ��� ����
	void save();

	size_t getClasterCount() const;

	//������ true, ���� ������ ��� �������, ����� false
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