#pragma once

#include <memory>
#include <span>

namespace Registry
{
	class IDataType
	{
	public:
		IDataType(const wchar_t* name);
		virtual ~IDataType();

		// non-copyable
		IDataType(const IDataType&) = delete;
		IDataType& operator=(const IDataType&) = delete;
		// non-movable
		IDataType(IDataType&&) = delete;
		IDataType& operator=(IDataType&&) = delete;

		static const IDataType* findDataType(const wchar_t* name);

	private:
		IDataType* prev;
		IDataType* next;
		static IDataType* head;
		static IDataType* last;
		const wchar_t* typeName;
	};

	/**
	 * Platform-specific key
	 */
	class IKey
	{
	public:
		virtual ~IKey() = default;

		virtual const wchar_t* getKeyName() const = 0;
	};
	using IKeyPtr = std::shared_ptr<IKey>;

	/**
	 * Platform-specific value
	 */
	class IValue
	{
	public:
		IValue();
		virtual ~IValue();

		virtual const wchar_t* getValueName() const = 0;
		virtual const IDataType& getType() const = 0;

		virtual std::span<const unsigned char> getRawData() const = 0;
		virtual void setRawData(const std::span<const unsigned char>& data) = 0;
	};
	using IValuePtr = std::shared_ptr<IValue>;
}
