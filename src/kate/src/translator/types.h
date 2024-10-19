#pragma once

#include <cstddef>
#include <string>
#include <memory>

#include "base/rtti.h"
#include "ast.h"

namespace kate::tlr::types {
  class Type : public base::rtti::Castable<Type, base::rtti::Base> {
  public:
    virtual ~Type() = default;

    virtual std::string mangledName() const = 0;

    virtual uint64_t numSlots() const { return 0; }
  };

  class Mat : public base::rtti::Castable<Mat, Type> {
  public:
    Mat() = delete;

    Mat(Type* type, size_t rows, size_t columns);

    size_t rows() const;

    size_t columns() const;

    Type* type();

    std::string mangledName() const override;

    uint64_t numSlots() const override;
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

    std::string mangledName() const override;
  private:
    Type* m_type;
    size_t m_count;
  };

  class Scalar : public base::rtti::Castable<Scalar, Type> {
  public:
    Scalar() = delete;

    Scalar(const std::string& name);

    std::string mangledName() const override;

    uint64_t numSlots() const override;
  private:
    std::string m_name;
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

    std::string mangledName() const override;
  private:
    std::string m_name;
    std::vector<Member> m_members;
  };

  class Mgr {
  public:
    Mgr();

    Type* findType(const std::string& name);
    
    types::Type* addType(
      const std::string& name,
      std::unique_ptr<Type>&& type
    );
  private:
    std::unordered_map<std::string, std::unique_ptr<Type>> m_type_table;
  };

  Mgr& system();
}