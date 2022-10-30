/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
/** \file etag.h
 * \brief Simple wrapper around meta data items.
 *
 * Most of meta data related items are stored in an associated MD block
 * which is an XML snippet. The items are stored in 'e' and 'tree' tags.
 */
#pragma once

#include <sstream>
#include <string>
#include <vector>
namespace mdf {


enum class ETagDataType : uint8_t {
  StringType = 0,
  DecimalType = 1,
  IntegerType = 2,
  FloatType = 3,
  BooleanType = 4,
  DateType = 5,
  TimeType = 6,
  DateTimeType = 7
};

/** \class ETag etag.h "mdf/etag.h"
 * \brief Helper class for meta data items in an MDF file.
 *
 * Most of meta data related items are stored in an associated MD block
 * which is an XML snippet. The items are stored in 'e' and 'tree' tags.
 *
 * The 'tree' tag is a list of 'e' tags and shall have a unique name attribute
 * and optional description and creator index.
 *
 * The 'e' tag shall have a unique name attribute and a value. The other
 * attributes are optional.
 */
class ETag {
 public:
  /** \brief Sets the name attribute in an e-tag or a tree-tag.
   *
   * The name attribute  should be unique making it
   * possible to search for. It is not possible to store if the name is blank.
   * @param name Unique name and not empty.
   */
  void Name(const std::string& name) { name_ = name; }

  /** \brief Returns the name attribute
   *
   * @return Name attribute
   */
  [[nodiscard]] const std::string& Name() const { return name_; }

  /** \brief Optional attribute in an e-tag or tree-tag.
   *
   * @param desc Description text (UTF8)
   */
  void Description(const std::string& desc) { desc_ = desc; }

  /** \brief Description text
   *
   * @return Description text (UTF8)
   */
  [[nodiscard]] const std::string& Description() const { return desc_; }

  /** \brief Optional unit of the value in a nn e-tag..
   *
   * @param unit Unit text (UTF8)
   */
  void Unit(const std::string& unit) { unit_ = unit; }

  /** \brief Unit of value.
   *
   * @return Unit of value (UTF8)
   */
  [[nodiscard]] const std::string& Unit() const { return unit_; }

  /** \brief Reference unit
   *
   * Reference to a global unit. This is an advanced MDF4 feature.
   * @param unit_ref Reference to a global unit.
   */
  void UnitRef(const std::string& unit_ref) { unit_ref_ = unit_ref; }

  /** \brief Reference unit
   *
   * @return Reference unit
   */
  [[nodiscard]] const std::string& UnitRef() const { return unit_ref_; }

  void DataType(ETagDataType type);
  [[nodiscard]] ETagDataType DataType() const;
  
  /** \brief Data type of the value.
   *
   * The data type is default set to a string. Standard types are according to
   * the XML standard primitive types. Standard types are 'string', 'decimal',
   * 'integer', 'float', 'boolean', 'date', 'time' and 'dateTime'.
   * @param type Data type string
   */
  void Type(const std::string& type) { type_ = type; }

  /** \brief Data type of the value.
   *
   * Data types as defined in 'Primitive XML Data Types'.
   * @return Value data type.
   */
  [[nodiscard]] const std::string& Type() const { return type_; }

  /** \brief Language of the value.
   *
   * Defines the language. Advanced feature.
   * @param language Language string
   */
  void Language(const std::string& language) { language_ = language; }

  /** \brief Language code.
   *
   * @return Language code.
   */
  [[nodiscard]] const std::string& Language() const { return language_; }

  /** \brief The value is read-only.
   *
   * @param read_only Set to true if the value is read-only.
   */
  void ReadOnly(bool read_only) { read_only_ = read_only; }

  /** \brief Indicates that the value is read-only.
   *
   * @return True if the value is read-only.
   */
  [[nodiscard]] bool ReadOnly() const { return read_only_; }

  /** \brief Index to FH block.
   *
   * Default set to -1 but can be set to which an index to a FH block indicating
   * who set the value in the block.
   * @param index Index to file history block.
   */
  void CreatorIndex(int index) { creator_index_ = index; }

  /** \brief Index to file history block
   *
   * @return Index to a file history block.
   */
  [[nodiscard]] int CreatorIndex() const { return creator_index_; }

  /** \brief Sets the value for an e-tag.
   *
   * @tparam T Type of value.
   * @param value Tag value.
   */
  template <typename T>
  void Value(const T& value);

  /** \brief Returns the tag value.
   *
   * @tparam T Type of value.
   * @return Tag value.
   */
  template <typename T>
  [[nodiscard]] T Value() const;

  /** \brief Adds a tag and define this to be a list of tags (tree).
   *
   * Adds a tag meaning that this tag should be treated as a list of tags
   * (tree).
   * @param tag Tag item to add
   */
  void AddTag(const ETag& tag);

  /** \brief Return a list of tags.
   *
   * @return List of tag items.
   */
  [[nodiscard]] const std::vector<ETag>& TreeList() const { return tree_list_; }

 private:
  std::string name_;
  std::string desc_;
  std::string unit_;
  std::string unit_ref_;
  std::string type_;
  std::string language_;
  std::string value_;
  bool read_only_ = false;
  int creator_index_ = -1;

  std::vector<ETag> tree_list_;
};

template <typename T>
void ETag::Value(const T& value) {
  std::ostringstream temp;
  temp << value;
  value_ = temp.str();
}

template <>
void ETag::Value(const bool& value);

template <typename T>
T ETag::Value() const {
  T temp_value = {};
  if (!value_.empty()) {
    std::istringstream temp(value_);
    temp >> temp_value;
  }
  return temp_value;
}

template <>
[[nodiscard]] bool ETag::Value() const;
}  // namespace mdf