#pragma once
#include "BinaryFile.h"
#include <map>
#include <iostream>
#include "list.h"
#include "CLMemory.h"


namespace CL 
{
	enum class BinaryTableElementType
	{
		_null = 0,
		_string = 1,
		_float  = 2,
		_int = 3,
		_charBuffer = 4,
		_bool = 5,
		_record = 6,
	};

	class BinaryTableRecord;

	struct CharBuffer
	{
		char* mData = nullptr;
		size_t size = 0;

		template<class Obj>CharBuffer(const Obj& obj)
		{
			size = sizeof(Obj);
			mData = CL_NEW_ARR( char, size);
			memcpy(mData, &obj, size);
		} 
		CharBuffer(const void* pData, size_t size);
		CharBuffer();
		CharBuffer(const CharBuffer& buffer); 
		CharBuffer& operator = (const CharBuffer& buffer)
		{
			if (mData)
			{
				CL_DELETE_ARR(mData);
			}

			size = buffer.size;

			if (size)
			{
				mData = CL_NEW_ARR( char, size);

				memcpy(mData, buffer.mData, size);
			}

			return *this;
		}
		template<class Obj> const Obj& cast() const
		{
			assert(sizeof(Obj) == size);
			return *reinterpret_cast<Obj*>(mData);
		}
		~CharBuffer();
	};

	

	class RecordCell
	{
		friend class BinaryTableRef;
		friend class BinaryListOutput;
		friend class BinaryListInput;
		friend void printCellData(const RecordCell& cell, const std::string offset);
	public:
		
		RecordCell(const std::string& cellName);

		RecordCell(const std::string& cellName, std::string& str);

		RecordCell(const std::string& cellName, const char* str);

		RecordCell(const std::string& cellName, float value);

		RecordCell(const std::string& cellName, int value);

		RecordCell(const std::string& cellName, const BinaryTableRecord& value);

		RecordCell(const std::string& cellName, const CharBuffer& buffer);

		RecordCell(const std::string& cellName, const bool& b);

		RecordCell(const RecordCell& cell);

		RecordCell& operator = (const std::string& str)
		{
			_bValueChanged = true;

			clearData();
			data = CL_NEW( std::string, str);

			type = BinaryTableElementType::_string;

			return *this;
		}

		RecordCell& operator = (const char* str)
		{
			_bValueChanged = true;

			clearData();
			data = CL_NEW(std::string, str);

			type = BinaryTableElementType::_string;

			return *this;
		}

		RecordCell& operator = (const CharBuffer& buffer)
		{
			_bValueChanged = true;

			clearData();

			data = CL_NEW( CharBuffer, buffer);
			type = BinaryTableElementType::_charBuffer; 

			return *this;
		}

		RecordCell& operator = (const bool& b)
		{
			_bValueChanged = true;

			clearData();

			data = CL_NEW( bool, b);
			type = BinaryTableElementType::_bool;

			return *this;
		}

		RecordCell& operator = (float value)
		{
			_bValueChanged = true;

			clearData();
			data = CL_NEW(float,value);

			type = BinaryTableElementType::_float;

			return *this;
		}

		RecordCell& operator = (int value)
		{
			_bValueChanged = true;

			clearData();
			data = CL_NEW( int,value);

			type = BinaryTableElementType::_int;

			return *this;
		}

		RecordCell& operator = (const BinaryTableRecord& value)
		{
			_bValueChanged = true;

			copyRecord(value);

			return *this;
		}

		RecordCell& operator = (const RecordCell& cell)
		{
			_bValueChanged = true;

			copyData(cell);

			return *this;
		}

		bool isNull() const;

		bool isString() const;

		bool isBool() const;

		bool isInt() const;

		bool isFloat() const;

		bool isCharBuffer() const;

		bool isRecord() const;

		const BinaryTableRecord& toRecord() const;

		const CharBuffer& toCharBuffer() const;

		const std::string& toString() const;

		bool toBool() const;

		int toInt() const;

		float toFloat() const;

		float toFloat(float fDefaultValue) const;

		~RecordCell();

		const std::string& getName() const;

		bool onChange() const;

	private:
		bool _bValueChanged = false;
		 
		std::string _cellName;

		BinaryTableElementType type = BinaryTableElementType::_null;
		void* data = nullptr;

		void clearData();

		void copyData(const RecordCell& cell);

		void copyRecord(const BinaryTableRecord& value);
	};

	class BinaryTableRecord
	{
		friend class BinaryList;
	public:
		
		BinaryTableRecord();

		RecordCell& operator [] (const std::string& name)
		{
			auto cellIt = mData.find(name);

			if (cellIt != mData.end())
			{
				return *cellIt->second;
			}

			RecordCell* cell = CL_NEW( RecordCell, name);

			mData.insert(std::make_pair(name, cell)).second;

			_bOnChange = true;

			return *mData.find(name)->second;
		}

		RecordCell& operator [] (const std::string& name) const 
		{
			auto cellIt = mData.find(name);

			assert(cellIt != mData.end());

			return *cellIt->second;
		}

		std::map<std::string, RecordCell*> mData;

		~BinaryTableRecord();

		bool exist(const std::string& name) const;

		void clear();

		BinaryTableRecord& operator = (const BinaryTableRecord&) = delete;
		
		BinaryTableRecord(const BinaryTableRecord& record);

		bool onChange() const;

	private: 
		bool _bOnChange = false;
	};
	 
	void printRecordData(const BinaryTableRecord& record, const std::string& offset = "");

	void printCellData(const RecordCell& cell, const std::string offset = "");

	class BinaryListOutput
	{
	public:

		BinaryListOutput(const std::string& file);
		 
		void write(const BinaryTableRecord& record);

		static void write(const BinaryTableRecord& record, BinaryOutputFile& _file);
 
	private:
		BinaryOutputFile  _file;

		static void write(const std::string& name, const RecordCell* cell, BinaryOutputFile& _file);
	}; 

	class BinaryListInput
	{
	public: 
		BinaryListInput(const std::string& file); 
		static void read(BinaryTableRecord& record, BinaryInputFile& _file); 
		~BinaryListInput();

		List<BinaryTableRecord*> record;
	private:
		BinaryInputFile _file;

		static RecordCell* read(BinaryInputFile& _file);

	};
}

