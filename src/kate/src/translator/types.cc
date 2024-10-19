#include "types.h"

#include <fmt/format.h>

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

  std::string Mat::mangledName() const
  {
    return fmt::format("{}{}x{}", m_type->mangledName(), rows(), columns()); 
  }

  uint64_t Mat::numSlots() const
  {
    return m_type->numSlots() * m_rows * m_columns;
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

  std::string Array::mangledName() const
  {
    return fmt::format("{}[{}]", m_type->mangledName(), count()); 
  }

  Custom::Member::Member(Type* type, const std::string& name)
    : m_type { type },
      m_name { name }
  {
  }

  std::string Custom::mangledName() const
  {
    return m_name; 
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

  Mgr::Mgr()
  {
    m_type_table["half"] = std::make_unique<types::Scalar>("half");
    
    for (auto i = 2; i <= 4; i++) {
      auto name = fmt::format("half{}", i);
      m_type_table[name] = std::make_unique<types::Vec>(m_type_table["half"].get(), i);

      for (auto j = 2; j <= 4; j++) {
        auto name = fmt::format("half{}x{}", i, j);
        m_type_table[name] = std::make_unique<types::Mat>(m_type_table["half"].get(), j, i);
      }
    }

    m_type_table["uhalf"] = std::make_unique<types::Scalar>("uhalf");
    
    for (auto i = 2; i <= 4; i++) {
      auto name = fmt::format("uhalf{}", i);
      m_type_table[name] = std::make_unique<types::Vec>(m_type_table["uhalf"].get(), i);

      for (auto j = 2; j <= 4; j++) {
        auto name = fmt::format("uhalf{}x{}", i, j);
        m_type_table[name] = std::make_unique<types::Mat>(m_type_table["uhalf"].get(), j, i);
      }
    }

    m_type_table["float"] = std::make_unique<types::Scalar>("float");
    
    for (auto i = 2; i <= 4; i++) {
      auto name = fmt::format("float{}", i);
      m_type_table[name] = std::make_unique<types::Vec>(m_type_table["float"].get(), i);

      for (auto j = 2; j <= 4; j++) {
        auto name = fmt::format("float{}x{}", i, j);
        m_type_table[name] = std::make_unique<types::Mat>(m_type_table["float"].get(), j, i);
      }
    }

    m_type_table["double"] = std::make_unique<types::Scalar>("double");
    
    for (auto i = 2; i <= 4; i++) {
      auto name = fmt::format("double{}", i);
      m_type_table[name] = std::make_unique<types::Vec>(m_type_table["double"].get(), i);

      for (auto j = 2; j <= 4; j++) {
        auto name = fmt::format("double{}x{}", i, j);
        m_type_table[name] = std::make_unique<types::Mat>(m_type_table["double"].get(), j, i);
      }
    }

    m_type_table["uint"] = std::make_unique<types::Scalar>("uint");
    
    for (auto i = 2; i <= 4; i++) {
      auto name = fmt::format("uint{}", i);
      m_type_table[name] = std::make_unique<types::Vec>(m_type_table["uint"].get(), i);

      for (auto j = 2; j <= 4; j++) {
        auto name = fmt::format("uint{}x{}", i, j);
        m_type_table[name] = std::make_unique<types::Mat>(m_type_table["uint"].get(), j, i);
      }
    }

    m_type_table["int"] = std::make_unique<types::Scalar>("int");
    
    for (auto i = 2; i <= 4; i++) {
      auto name = fmt::format("int{}", i);
      m_type_table[name] = std::make_unique<types::Vec>(m_type_table["int"].get(), i);

      for (auto j = 2; j <= 4; j++) {
        auto name = fmt::format("int{}x{}", i, j);
        m_type_table[name] = std::make_unique<types::Mat>(m_type_table["int"].get(), j, i);
      }
    }
  }

  types::Type* Mgr::findType(const std::string& type)
  {
    auto it = m_type_table.find(type);

    return (it != m_type_table.end()) ? it->second.get() : nullptr;
  }

  types::Type* Mgr::addType(
    const std::string& name,
    std::unique_ptr<Type>&& type
  )
  {
    auto ptr = type.get();
    m_type_table[name] = std::move(type);
    return ptr;
  }

  Mgr& system()
  {
    static Mgr mgr;
    return mgr;
  }

  Vec::Vec(Type* type, size_t columns)
    : m_type { type },
      m_columns { columns }
  {
  }

  size_t Vec::columns() const
  {
    return m_columns;
  }

  Type* Vec::type()
  {
    return m_type;
  }

  std::string Vec::mangledName() const
  {
    return fmt::format("{}{}", m_type->mangledName(), m_columns);
  }

  uint64_t Vec::numSlots() const
  {
    return m_type->numSlots() * m_columns;
  }

  Scalar::Scalar(const std::string& name) : m_name { name }
  {
  }

  std::string Scalar::mangledName() const
  {
    return m_name;
  }

  uint64_t Scalar::numSlots() const
  {
    return 1;
  }
}

TS_RTTI_TYPE(kate::tlr::types::Type)
TS_RTTI_TYPE(kate::tlr::types::Mat)
TS_RTTI_TYPE(kate::tlr::types::Array)
TS_RTTI_TYPE(kate::tlr::types::Vec)
TS_RTTI_TYPE(kate::tlr::types::Custom)
TS_RTTI_TYPE(kate::tlr::types::Scalar)