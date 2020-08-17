#include "BinaryDB.h"
 
size_t DataDecriptor::getSize() const
{
	return _nBytes;
}

const std::string& DataDecriptor::getName() const
{
	return _sName;
}

DataFileObject::DataFileObject(const BinaryDB* pOwner, const std::string& sName) :
	_pOwner(pOwner), _sName(sName)
{

}

const std::string& DataFileObject::getName() const
{
	return _sName;
}

const BinaryDB* DataFileObject::getOwner() const
{
	return _pOwner;
}

const DataDecriptor* DataFileObject::getFile(const std::string& file) const
{
	auto it = _localFileMap.find(file);

	DataDecriptor* pDataDesc = nullptr;

	if (it != _localFileMap.end())
	{
		pDataDesc = it->second;
	}

	return pDataDesc;
}

DataFileObject::~DataFileObject()
{
	for (auto it : _localFileMap)
	{
		delete it.second;
	}
}

bool DataFileObject::onChange() const
{
	bool bChangeStatus = _bOnChange;

	if (!bChangeStatus)
	{
		bChangeStatus |= objectFild.onChange();
	}

	return bChangeStatus;
}

BinaryDB::BinaryDB(const std::string& sFolder, const std::string& sFileName, bool bOverride)
{
	_sName = sFileName;

	_manifestFilePatch = sFolder + '/' + sFileName + ".bman";

	int mode = bOverride ? std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc : std::ios::binary | std::ios::in | std::ios::out;

	_manifestFile.open(_manifestFilePatch, mode);

	if (!_manifestFile.is_open())
	{
		_manifestFile.open(_manifestFilePatch, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);

		_bOnChange = true;

		if (!_manifestFile.is_open())
		{
			std::string message = "Fail to open " + _manifestFilePatch;

			TException(message.c_str());
		}

		_manifestFile.close();
	}
	else
	{
		_manifestFile.close();

		if (!bOverride)
		{
			readManifest();
		}
		else
		{
			_bOnChange = true;
		}

	}


	_dataFilePatch = sFolder + '/' + sFileName + ".bdat";

	_dataFile.open(_dataFilePatch, mode);

	if (!_dataFile.is_open())
	{
		_dataFile.open(_dataFilePatch, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);

		if (!_dataFile.is_open())
		{
			std::string message = "Fail to open " + _dataFilePatch;

			TException(message.c_str());
		}
	}
}

bool BinaryDB::erase(DataFileObject* pDataObject, const std::string& sFileName)
{
	auto it = pDataObject->_localFileMap.find(sFileName);

	if (it == pDataObject->_localFileMap.end() || pDataObject->_pOwner != this)
	{
		return false;
	}

	auto pNode = it->second->_pFirstNode;

	while (pNode)
	{
		if (!pNode->pNext)
		{
			pNode->pNext = pFirstFreeClaster;
			pNode = nullptr;
		}
		else
		{
			pNode = pNode->pNext;
		}
	}

	pFirstFreeClaster = it->second->_pFirstNode;

	pDataObject->_localFileMap.erase(it);

	_bOnChange = true;

	return true;
}
 
bool BinaryDB::erase(const std::string& sName)
{
	auto it = _manifestMap.find(sName);

	if (it == _manifestMap.end())
	{
		return false;
	}

	auto fileMap = it->second->_localFileMap;

	for (auto file : fileMap)
	{
		erase(it->second, file.second->_sName);
	}

	_manifestMap.erase(it);
	delete it->second;

	_bOnChange = true;

	return true;
}
 
bool BinaryDB::rename(const std::string& sOldName, const std::string& sNewName)
{
	bool bResult = false;

	auto oldIt = _manifestMap.find(sOldName);
	auto newIt = _manifestMap.find(sNewName);

	if (oldIt != _manifestMap.end() && newIt == _manifestMap.end())
	{
		auto pManifest = oldIt->second;
		pManifest->_sName = sNewName;

		_manifestMap.erase(oldIt);

		_manifestMap.insert(std::make_pair(sNewName, pManifest));
		
		_bOnChange = true;
		bResult = true;
	}

	

	return bResult;
}
 
bool BinaryDB::exist(const std::string& sName) const
{
	return _manifestMap.find(sName) != _manifestMap.end();
}

bool BinaryDB::write(DataFileObject* pDataObject, const std::string& sFileName, char* pData, size_t nDataSize)
{
	if (!pDataObject || this != pDataObject->getOwner() || !sFileName.length() || !nDataSize || pDataObject->_localFileMap.find(sFileName) != pDataObject->_localFileMap.end())
	{
		return false;
	}

	Compress::CompressedData data;

	if (!Compress::compressData((const char*)pData, nDataSize, data))
	{
		TException("Критическая ошибка: не получилось сжать данные");
	}


	size_t nBlockNeeded = data.nCompressedBytes / FILE_CLASTER_SIZE + (data.nCompressedBytes % FILE_CLASTER_SIZE ? 1 : 0);
	size_t nBlocAvalible = getFreeClasterCount();

	if (nBlocAvalible < nBlockNeeded)
	{
		alocate(nBlockNeeded - nBlocAvalible);
	}

	auto pNode = pFirstFreeClaster;

	DataDecriptor* pDescriptor = new DataDecriptor();
	pDescriptor->_nCompressBytes = data.nCompressedBytes;
	pDescriptor->_nBytes = nDataSize;
	pDescriptor->_sName = sFileName;
	pDescriptor->_pFirstNode = pNode;
	pDescriptor->_pOwner = this;

	size_t nRemaningBytes = data.nCompressedBytes;

	char* pWriteData = data.pCompressedData;

	while (nBlockNeeded)
	{
		size_t nWriteBytes = FILE_CLASTER_SIZE > nRemaningBytes ? nRemaningBytes : FILE_CLASTER_SIZE;

		if (!pNode->pData)
		{
			pNode->pData = new char[FILE_CLASTER_SIZE];
		}

		memcpy(pNode->pData, pWriteData, nWriteBytes);

		if (nWriteBytes < FILE_CLASTER_SIZE)
		{
			for (size_t i = nWriteBytes; i < FILE_CLASTER_SIZE; i++)
			{
				pNode->pData[i] = 0;
			}
		} 

		nRemaningBytes -= nWriteBytes;
		pWriteData     += nWriteBytes;
		 
		if (nBlockNeeded == 1)
		{
			pFirstFreeClaster = pNode->pNext;
			pNode->pNext = nullptr;
		}
		else
		{
			pNode = pNode->pNext;
		}
		
		nBlockNeeded--; 
	} 

	pDataObject->_localFileMap.insert(std::make_pair(pDescriptor->_sName, pDescriptor));

	_bOnChange = true;

	return true;
}

bool BinaryDB::read(const DataDecriptor* pDescriptor, char* pData)
{
	if (pDescriptor->_pOwner != this)
	{
		return false;
	}

	size_t nRemaningBytes = pDescriptor->_nCompressBytes;

	Compress::CompressedData data;
	data.nBytes = pDescriptor->_nBytes;
	data.nCompressedBytes = pDescriptor->_nCompressBytes;
	data.pCompressedData = new char[pDescriptor->_nCompressBytes];

	char* pCompressData = data.pCompressedData;



	ClasterIterator it;
	it.pCurrentClaster = pDescriptor->_pFirstNode;

	while (it.iterate())
	{
		size_t iItBytes = it.iClasterCount * FILE_CLASTER_SIZE;
		size_t nReadBytes = nRemaningBytes > iItBytes ? iItBytes : nRemaningBytes;

		if (it.bReadFromMem)
		{
			memcpy(pCompressData, it.pData, nReadBytes);
		}
		else
		{
			_dataFile.seekg(it.iPosition);
			_dataFile.read(pCompressData, nReadBytes);
		}

		nRemaningBytes -= nReadBytes;
		pCompressData  += nReadBytes;

	}

	size_t size = pDescriptor->_nBytes;

	if (!Compress::uncompressData(pData, size, data))
	{
		TException("Критическая ошибка: не получилось разжать данные");
	}

	return true;
}

DataFileObject* BinaryDB::createFileManifest(const std::string& sName)
{
	auto it = _manifestMap.find(sName);
	DataFileObject* pManifest = nullptr;

	if (it == _manifestMap.end())
	{
		pManifest = new DataFileObject(this, sName);

		_manifestMap.insert(std::make_pair(sName, pManifest));
	
		_bOnChange = true;
	} 

	return pManifest;
}

//Вернет указатель на объект данных, если он уже существует, иначе он вернет nullptr

DataFileObject* BinaryDB::findFileManifest(const std::string& sName)
{
	auto it = _manifestMap.find(sName);
	DataFileObject* pManifest = nullptr;

	if (it != _manifestMap.end())
	{
		pManifest = it->second;

	}

	return pManifest;
}
 
BinaryDB::~BinaryDB()
{
	for (decltype(auto) claster : _mClaster)
	{
		if (claster->pData)
		{
			delete[] claster->pData;
		}
	}

	for (auto it : _manifestMap)
	{
		delete it.second;
	}

	for (auto pClaster : _mClaster)
	{
		delete pClaster;
	}
}

void BinaryDB::save()
{
	if (!onChange())
	{
		return;
	}

	_bOnChange = false;

	CL::BinaryOutputFile manifestOutputFile(_manifestFilePatch);

	manifestOutputFile.write(_sName);

	if (pFirstFreeClaster)
	{
		manifestOutputFile.write(pFirstFreeClaster->iLocalId);
	}
	else
	{
		manifestOutputFile.write(size_t(-1));
	}

	manifestOutputFile.write(_mClaster.size());

	for (auto pClaster : _mClaster)
	{
		if (pClaster->pNext)
		{
			manifestOutputFile.write(pClaster->pNext->iLocalId);
		}
		else
		{
			manifestOutputFile.write(size_t(-1));
		}
	}

	manifestOutputFile.write(_manifestMap.size());

	for (auto it : _manifestMap)
	{
		manifestOutputFile.write(it.first);
		CL::BinaryListOutput::write(it.second->objectFild, manifestOutputFile);

		manifestOutputFile.write(it.second->_localFileMap.size());

		for (auto file : it.second->_localFileMap)
		{
			manifestOutputFile.write(file.second->_sName);
			manifestOutputFile.write(file.second->_pFirstNode->iLocalId);
			manifestOutputFile.write(file.second->_nBytes);
			manifestOutputFile.write(file.second->_nCompressBytes);
		}
	}

	static char tmdData[FILE_CLASTER_SIZE];

	_dataFile.seekg(0, _dataFile.end);
 
	while(_nAlocatedClasters--)
	{
		_dataFile.write(tmdData, FILE_CLASTER_SIZE);
	}; 

	for (decltype(auto) claster : _mClaster)
	{
		if (claster->pData)
		{
			_dataFile.seekg(claster->iLocalId * FILE_CLASTER_SIZE);
			_dataFile.write(claster->pData, FILE_CLASTER_SIZE);

			DELETE_ARRAY_PTR(claster->pData);
		} 
	}



}

size_t BinaryDB::getClasterCount() const
{
	return _mClaster.size();
}

bool BinaryDB::onChange() const
{
	bool bChangeStatus = _bOnChange;

	if (!bChangeStatus)
	{
		for (auto it : _manifestMap)
		{
			bChangeStatus = it.second->onChange();

			if (bChangeStatus)
			{
				break;
			}
		}
	}

	return bChangeStatus;
}

size_t BinaryDB::getFreeClasterCount() const
{
	size_t count = 0;

	auto pClaster = pFirstFreeClaster;

	while (pClaster)
	{
		count++;
		pClaster = pClaster->pNext;
	}

	return count;
}

void BinaryDB::alocate(size_t nBlock)
{
	_nAlocatedClasters += nBlock;

	size_t iStartPosition = _mClaster.size();

	_mClaster.resize(_mClaster.size() + nBlock);
 
	for (size_t i = nBlock; i > 0; i--)
	{
		FileClaster* pClaster = new FileClaster();
		pClaster->iLocalId = iStartPosition + i - 1; 

		_mClaster[pClaster->iLocalId] = pClaster;

		pClaster->pNext = pFirstFreeClaster;
		pFirstFreeClaster = pClaster;
 
	} 
}

void BinaryDB::readManifest()
{
	CL::BinaryInputFile manifestInputFile(_manifestFilePatch);

	manifestInputFile.read(_sName);

	size_t iFreeClasterId = -1;

	manifestInputFile.read(iFreeClasterId);

	size_t nClasterCount = 0;
	manifestInputFile.read(nClasterCount);

	_mClaster.resize(nClasterCount);

	for (size_t i = 0; i < nClasterCount; i++)
	{
		_mClaster[i] = new FileClaster();
		_mClaster[i]->iLocalId = i;
	}

	for (size_t i = 0; i < nClasterCount; i++)
	{
		FileClaster* pClaster = _mClaster[i];

		pClaster->iLocalId = i;


		size_t nextId = 0;

		manifestInputFile.read(nextId);

		if (nextId != -1)
		{
			pClaster->pNext = _mClaster[nextId];
		}

	}

	if (iFreeClasterId != -1)
	{
		pFirstFreeClaster = _mClaster[iFreeClasterId];
	}

	size_t nManifestFiles = 0;
	manifestInputFile.read(nManifestFiles);

	for (size_t i = 0; i < nManifestFiles; i++)
	{
		std::string name;

		manifestInputFile.read(name);

		DataFileObject* pData = new DataFileObject(this, name);


		CL::BinaryListInput::read(pData->objectFild, manifestInputFile);

		size_t localFileSize = 0;

		manifestInputFile.read(localFileSize);

		if (localFileSize)
		{
			for (size_t fileId = 0; fileId < localFileSize; fileId++)
			{
				size_t nodeId = 0;

				DataDecriptor* pDescriptor = new DataDecriptor();

				manifestInputFile.read(pDescriptor->_sName);
				manifestInputFile.read(nodeId);
				manifestInputFile.read(pDescriptor->_nBytes);
				manifestInputFile.read(pDescriptor->_nCompressBytes);

				pDescriptor->_pOwner = this;

				pDescriptor->_pFirstNode = _mClaster[nodeId];

				pData->_localFileMap.insert(std::make_pair(pDescriptor->_sName, pDescriptor));
			}
		}

		_manifestMap.insert(std::make_pair(name, pData));

	}

}

FileClaster::~FileClaster()
{
	if (pData)
	{
		delete[] pData;
	}
}

bool ClasterIterator::iterate()
{
	iClasterCount = 0;

	if (pCurrentClaster)
	{
		bReadFromMem = pCurrentClaster->pData;

		if (bReadFromMem)
		{
			iClasterCount = 1;
			pData = pCurrentClaster->pData;
			pCurrentClaster = pCurrentClaster->pNext;
		}
		else
		{
			iPosition = pCurrentClaster->iLocalId * FILE_CLASTER_SIZE;

			while (pCurrentClaster)
			{
				iClasterCount++;

				auto pNode = pCurrentClaster;

				pCurrentClaster = pCurrentClaster->pNext;

				if (pNode->pNext)
				{

					if (pNode->iLocalId + 1 != pNode->pNext->iLocalId)
					{
						break;
					}
				}
			}
		}

	}
	 
	return iClasterCount;
}
