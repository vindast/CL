#include "BinaryList.h"

CL::RecordCell::RecordCell(const std::string& cellName) :
	_cellName(cellName)
{

}

CL::RecordCell::RecordCell(const std::string& cellName, std::string& str) :
	_cellName(cellName)
{
	data = CL_NEW( std::string, str);

	type = BinaryTableElementType::_string;
	
	_bValueChanged = false;
}

CL::RecordCell::RecordCell(const std::string& cellName, const char* str) :
	_cellName(cellName)
{
	data = CL_NEW(std::string, str);

	type = BinaryTableElementType::_string;

	_bValueChanged = false;
}

CL::RecordCell::RecordCell(const std::string& cellName, float value) :
	_cellName(cellName)
{
	data = CL_NEW(float, value);

	type = BinaryTableElementType::_float;

	_bValueChanged = false;
}

CL::RecordCell::RecordCell(const std::string& cellName, int value) :
	_cellName(cellName)
{
	data = CL_NEW(int, value);

	type = BinaryTableElementType::_int;

	_bValueChanged = false;
}

CL::RecordCell::RecordCell(const std::string& cellName, const BinaryTableRecord& value) :
	_cellName(cellName)
{
	copyRecord(value);

	_bValueChanged = false;
}

CL::RecordCell::RecordCell(const std::string& cellName, const CharBuffer& buffer) :
	_cellName(cellName)
{
	data = CL_NEW(CharBuffer, buffer);

	type = BinaryTableElementType::_charBuffer;

	_bValueChanged = false;
}

CL::RecordCell::RecordCell(const std::string& cellName, const bool& b) :
	_cellName(cellName)
{
	data = CL_NEW(bool, b);

	type = BinaryTableElementType::_bool;

	_bValueChanged = false;
}

CL::RecordCell::RecordCell(const RecordCell& cell)
{
	*this = cell;
	_bValueChanged = true;
}

void CL::RecordCell::copyRecord(const BinaryTableRecord& value)
{
	clearData();
	data = CL_NEW(BinaryTableRecord, value);

	type = BinaryTableElementType::_record;

	_bValueChanged = false;
}

bool CL::RecordCell::isNull() const
{
	return type == BinaryTableElementType::_null;
}

bool CL::RecordCell::isString() const
{
	return type == BinaryTableElementType::_string;
}

bool CL::RecordCell::isBool() const
{
	return type == BinaryTableElementType::_bool;
}

bool CL::RecordCell::isInt() const
{
	return type == BinaryTableElementType::_int;
}

bool CL::RecordCell::isFloat() const
{
	return type == BinaryTableElementType::_float;
}

bool CL::RecordCell::isCharBuffer() const
{
	return type == BinaryTableElementType::_charBuffer;
}

bool CL::RecordCell::isRecord() const
{
	return type == BinaryTableElementType::_record;
}

const CL::BinaryTableRecord& CL::RecordCell::toRecord() const
{
	assert(type == BinaryTableElementType::_record);

	return *reinterpret_cast<BinaryTableRecord*>(data);
}

const CL::CharBuffer& CL::RecordCell::toCharBuffer() const
{
	assert(type == BinaryTableElementType::_charBuffer);

	return *reinterpret_cast<CharBuffer*>(data);
}

const std::string& CL::RecordCell::toString() const
{
	assert(type == BinaryTableElementType::_string);

	return *reinterpret_cast<std::string*>(data);
}

bool CL::RecordCell::toBool() const
{
	assert(type == BinaryTableElementType::_bool);

	return *reinterpret_cast<bool*>(data);
}

int CL::RecordCell::toInt() const
{
	assert(type == BinaryTableElementType::_int);

	return *reinterpret_cast<int*>(data);
}

float CL::RecordCell::toFloat() const
{
	assert(type == BinaryTableElementType::_float);

	return *reinterpret_cast<float*>(data);
}

float CL::RecordCell::toFloat(float fDefaultValue) const
{
	float fValue = fDefaultValue;

	if (type == BinaryTableElementType::_float)
	{
		fValue = *reinterpret_cast<float*>(data);
	}

	return fValue;
}

CL::RecordCell::~RecordCell()
{
	clearData();
}

const std::string& CL::RecordCell::getName() const
{
	return _cellName;
}

bool CL::RecordCell::onChange() const
{
	return _bValueChanged;
}

void CL::RecordCell::clearData()
{
	if (data)
	{
		CL_DELETE(data);
		data = nullptr;
	}
}

void CL::RecordCell::copyData(const CL::RecordCell& cell)
{
	clearData();


	switch (cell.type)
	{
	case CL::BinaryTableElementType::_null:

		break;
	case CL::BinaryTableElementType::_string:
		*this = cell.toString();
		break;
	case CL::BinaryTableElementType::_float:
		*this = cell.toFloat();
		break;
	case CL::BinaryTableElementType::_int:
		*this = cell.toInt();
		break;

	case CL::BinaryTableElementType::_bool:
		*this = cell.toBool();
		break;

	case CL::BinaryTableElementType::_charBuffer:
		*this = cell.toCharBuffer();
		break;

	case CL::BinaryTableElementType::_record:
		*this = cell.toRecord();
		break;

	default:
		assert(false);
		break;
	}
}

CL::BinaryListInput::BinaryListInput(const std::string& file) :
	_file(file)
{ 
	while (!_file.eof())
	{
		BinaryTableRecord* r = CL_NEW(BinaryTableRecord);

		read(*r, _file);
		 
		if (r->mData.size())
		{
			record.push_back(r);
		}
		else
		{
			CL_DELETE(r);
		} 
	} 
}

void CL::BinaryListInput::read(BinaryTableRecord& record, BinaryInputFile& _file)
{
	size_t size = 0;

	_file.read(size);

	for (size_t i = 0; i < size; i++)
	{
		RecordCell* cell = read(_file);

		record.mData.insert(std::make_pair(cell->getName(), cell));
	}
}

CL::BinaryListInput::~BinaryListInput()
{
	for (auto it = record.begin(); it != record.end(); it++)
	{
		CL_DELETE(it());
	}
}

CL::RecordCell* CL::BinaryListInput::read(BinaryInputFile& _file)
{
	std::string name;

	_file.read(name);

	RecordCell* cell = CL_NEW( RecordCell, name);

	_file.read(cell->type);

	std::string s;
	float f = 0.0f;
	int i = 0;
	bool b = false;

	switch (cell->type)
	{
	case CL::BinaryTableElementType::_null:

		break;
	case CL::BinaryTableElementType::_string:
		_file.read(s);
		*cell = s;
		break;
	case CL::BinaryTableElementType::_float:
		_file.read(f);
		*cell = f;
		break;
	case CL::BinaryTableElementType::_int:
		_file.read(i);
		*cell = i;
		break;

	case CL::BinaryTableElementType::_bool:
		_file.read(b);
		*cell = b;
		break;

	case CL::BinaryTableElementType::_charBuffer:
	{
		CharBuffer buffer;
		_file.read(buffer.size);

		if (buffer.size)
		{
			buffer.mData = CL_NEW_ARR(char, buffer.size);
			_file.read(buffer.mData, buffer.size);
		}

		*cell = buffer;
		break;
	}
	case CL::BinaryTableElementType::_record:
	{
		BinaryTableRecord record;

		read(record, _file);

		*cell = record;

		break;
	}
	default:
		assert(false);
		break;
	}

	return cell;
}

CL::BinaryListOutput::BinaryListOutput(const std::string& file) :
	_file(file)
{

}

void CL::BinaryListOutput::write(const BinaryTableRecord& record)
{
	write(record, _file);
}

void CL::BinaryListOutput::write(const BinaryTableRecord& record, BinaryOutputFile& _file)
{
	_file.write(record.mData.size());
	 
	for (auto it = record.mData.begin(); it != record.mData.end(); it++)
	{
		write(it->first, it->second, _file);
	}
}

void CL::BinaryListOutput::write(const std::string& name, const RecordCell* cell, BinaryOutputFile& _file)
{
	_file.write(name);
	_file.write(cell->type);

	switch (cell->type)
	{
	case CL::BinaryTableElementType::_null:

		break;
	case CL::BinaryTableElementType::_string:
		_file.write(*reinterpret_cast<std::string*>(cell->data));
		break;
	case CL::BinaryTableElementType::_float:
		_file.write(*reinterpret_cast<float*>(cell->data));
		break;
	case CL::BinaryTableElementType::_int:
		_file.write(*reinterpret_cast<int*>(cell->data));
		break;
	case CL::BinaryTableElementType::_bool:
		_file.write(*reinterpret_cast<bool*>(cell->data));
		break;
	case CL::BinaryTableElementType::_charBuffer:
	{
		CharBuffer* buffer = reinterpret_cast<CharBuffer*>(cell->data);
		_file.write(buffer->size);

		if (buffer->size)
		{
			_file.write(buffer->mData, buffer->size);
		}

		break;
	}

	case CL::BinaryTableElementType::_record:
		write(*reinterpret_cast<BinaryTableRecord*>(cell->data), _file);
		break;

	default:
		assert(false);
		break;
	}

}

void CL::printRecordData(const CL::BinaryTableRecord& record, const std::string& offset)
{
	std::cout << offset << "fild count = " << record.mData.size() << std::endl;

	for (auto it = record.mData.begin(); it != record.mData.end(); it++)
	{
		std::cout << offset << "fild = " << it->first << ", ";

		printCellData(*it->second, offset);
	}

	std::cout << std::endl;
}

void CL::printCellData(const CL::RecordCell& cell, const std::string offset)
{
	//	std::cout << "name = " << _cellName << ", ";

	std::cout << offset;

	switch (cell.type)
	{
	case CL::BinaryTableElementType::_null:
		std::cout << "Type = null, data = " << cell.data << std::endl;
		break;
	case CL::BinaryTableElementType::_string:
		std::cout << "Type = string, data = " << *reinterpret_cast<std::string*>(cell.data) << std::endl;
		break;
	case CL::BinaryTableElementType::_float:
		std::cout << "Type = float, data = " << *reinterpret_cast<float*>(cell.data) << std::endl;
		break;
	case CL::BinaryTableElementType::_int:
		std::cout << "Type = int, data = " << *reinterpret_cast<int*>(cell.data) << std::endl;
		break;
	case CL::BinaryTableElementType::_charBuffer:
	{
		CharBuffer* buffer = reinterpret_cast<CharBuffer*>(cell.data);

		std::cout << "Type = CharBuffer, data = " << buffer->size << ", data = ";

		for (size_t i = 0; i < buffer->size; i++)
		{
			std::cout << buffer->mData[i];

			if (i < buffer->size - 1)
			{
				std::cout << ", ";
			}
		}

		std::cout << std::endl;

	}
	break;

	case CL::BinaryTableElementType::_record:
		std::cout << "Type = record" << std::endl;

		printRecordData(*reinterpret_cast<BinaryTableRecord*>(cell.data), offset + "     ");

		break;

	case CL::BinaryTableElementType::_bool:
		std::cout << "Type = bool, data = " << *reinterpret_cast<bool*>(cell.data) << std::endl;
		break;
	}
}

CL::BinaryTableRecord::BinaryTableRecord()
{

}

CL::BinaryTableRecord::~BinaryTableRecord()
{
	clear();
}

bool CL::BinaryTableRecord::exist(const std::string& name) const
{
	return mData.find(name) != mData.end();
}

void CL::BinaryTableRecord::clear()
{
	for (auto it = mData.begin(); it != mData.end(); it++)
	{
		CL_DELETE(it->second);
	}

	mData.clear();

	_bOnChange = true;
}

CL::BinaryTableRecord::BinaryTableRecord(const BinaryTableRecord& record)
{
	clear();

	for (auto data : record.mData)
	{
		mData.insert(std::make_pair(data.first, CL_NEW( RecordCell,*data.second)));
	}
}

bool CL::BinaryTableRecord::onChange() const
{
	bool bOnChange = _bOnChange;

	if (!bOnChange)
	{
		for (auto it : mData)
		{
			bOnChange = it.second->onChange();

			if (bOnChange)
			{
				break;
			}
		}
	}

	return bOnChange;
}

CL::CharBuffer::CharBuffer(const void* pData, size_t size)
{
	this->size = size;
	mData = CL_NEW_ARR( char, size);

	memcpy(mData, pData, size);
}

CL::CharBuffer::CharBuffer()
{

}

CL::CharBuffer::CharBuffer(const CharBuffer& buffer)
{
	*this = buffer;
}

CL::CharBuffer::~CharBuffer()
{
	if (mData)
	{
		CL_DELETE_ARR(mData);
	}
}
