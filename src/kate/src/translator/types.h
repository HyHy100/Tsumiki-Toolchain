#pragma once

#include <cstddef>
#include <string>

#include "base/rtti.h"

namespace kate::tlr::types {
  class Type : public base::rtti::Castable<Type, base::rtti::Base> {
  public:
    virtual ~Type() = default;
  };

  class Mat : public base::rtti::Castable<Mat, Type> {
  public:
    Mat() = delete;

    Mat(Type* type, size_t rows, size_t columns);

    size_t rows() const;

    size_t columns() const;

    Type* type();
  private:
    Type* m_type;
    size_t m_rows;
    size_t m_columns;
  };

  class Array : public base::rtti::Castable<Array, Type> {
  public:
    Array() = delete;

    Array(Type* type, size_t count);

    size_t count() const;

    Type* type();
  private:
    Type* m_type;
    size_t m_count;
  };

  class Custom : public base::rtti::Castable<Custom, Type> {
  public:
    class Member { 
    public:
      Member() = delete;

      Member(Type* type, const std::string& name);

      Type* type();
    private:
      Type* m_type;
      std::string m_name;
    };
    
    Custom() = delete;

    Custom(
      const std::string& name,
      std::vector<Member>&& members
    );

    const std::string& name() const;

    std::vector<Member>& members();
  private:
    std::string m_name;
    std::vector<Member> m_members;
  };
}