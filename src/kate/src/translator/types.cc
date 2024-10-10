#include "types.h"

namespace kate::tlr::types {
  Mat::Mat(
    Type* type,
    size_t rows,
    size_t columns
  ) : m_rows { rows },
      m_columns { columns },
      m_type { type }
  {
  }

  size_t Mat::rows() const
  {
    return m_rows;
  }

  size_t Mat::columns() const
  {
    return m_columns;
  }

  Type* Mat::type()
  {
    return m_type;
  }

  Array::Array(Type* type, size_t count)
    : m_count { count },
      m_type { type }
  {
  }

  size_t Array::count() const
  {
    return m_count;
  }

  Type* Array::type()
  {
    return m_type;
  }

  Custom::Member::Member(Type* type, const std::string& name)
    : m_type { type },
      m_name { name }
  {
  }

  Type* Custom::Member::type()
  {
    return m_type;
  }

  Custom::Custom(
    const std::string& name,
    std::vector<Member>&& members
  ) : m_name{ name },
      m_members { std::move(members) }
  {
  }

  const std::string& Custom::name() const
  {
    return m_name;
  }

  std::vector<Custom::Member>& Custom::members()
  {
    return m_members;
  }
}

TS_RTTI_TYPE(kate::tlr::types::Type)
TS_RTTI_TYPE(kate::tlr::types::Mat)
TS_RTTI_TYPE(kate::tlr::types::Array)
TS_RTTI_TYPE(kate::tlr::types::Custom)