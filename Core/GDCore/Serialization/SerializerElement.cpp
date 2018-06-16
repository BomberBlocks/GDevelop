#include "GDCore/Serialization/SerializerElement.h"

#include <iostream>

namespace gd {

SerializerElement SerializerElement::nullElement;

SerializerElement::SerializerElement() : valueUndefined(true) {}

SerializerElement::SerializerElement(const SerializerValue& value)
    : valueUndefined(false), elementValue(value) {}

SerializerElement::~SerializerElement() {}

const SerializerValue& SerializerElement::GetValue() const {
  if (valueUndefined && attributes.find("value") != attributes.end())
    return attributes.find("value")->second;

  return elementValue;
}

SerializerElement& SerializerElement::SetAttribute(const gd::String& name,
                                                   bool value) {
  attributes[name].SetBool(value);
  return *this;
}

SerializerElement& SerializerElement::SetAttribute(const gd::String& name,
                                                   const gd::String& value) {
  attributes[name].SetString(value);
  return *this;
}

SerializerElement& SerializerElement::SetAttribute(const gd::String& name,
                                                   int value) {
  attributes[name].SetInt(value);
  return *this;
}

SerializerElement& SerializerElement::SetAttribute(const gd::String& name,
                                                   double value) {
  attributes[name].SetDouble(value);
  return *this;
}

bool SerializerElement::GetBoolAttribute(const gd::String& name,
                                         bool defaultValue,
                                         gd::String deprecatedName) const {
  if (attributes.find(name) != attributes.end()) {
    return attributes.find(name)->second.GetBool();
  } else if (!deprecatedName.empty() &&
             attributes.find(deprecatedName) != attributes.end()) {
    return attributes.find(deprecatedName)->second.GetBool();
  } else {
    if (HasChild(name, deprecatedName)) {
      SerializerElement& child = GetChild(name, 0, deprecatedName);
      if (!child.IsValueUndefined()) {
        return child.GetValue().GetBool();
      }
    }
  }

  std::cout << "Bool attribute \"" << name << "\" not found, returning "
            << defaultValue;
  return defaultValue;
}

gd::String SerializerElement::GetStringAttribute(
    const gd::String& name,
    gd::String defaultValue,
    gd::String deprecatedName) const {
  if (attributes.find(name) != attributes.end())
    return attributes.find(name)->second.GetString();
  else if (!deprecatedName.empty() &&
           attributes.find(deprecatedName) != attributes.end())
    return attributes.find(deprecatedName)->second.GetString();
  else {
    if (HasChild(name, deprecatedName)) {
      SerializerElement& child = GetChild(name, 0, deprecatedName);
      if (!child.IsValueUndefined()) return child.GetValue().GetString();
    }
  }

  return defaultValue;
}

int SerializerElement::GetIntAttribute(const gd::String& name,
                                       int defaultValue,
                                       gd::String deprecatedName) const {
  if (attributes.find(name) != attributes.end())
    return attributes.find(name)->second.GetInt();
  else if (!deprecatedName.empty() &&
           attributes.find(deprecatedName) != attributes.end())
    return attributes.find(deprecatedName)->second.GetInt();
  else {
    if (HasChild(name, deprecatedName)) {
      SerializerElement& child = GetChild(name, 0, deprecatedName);
      if (!child.IsValueUndefined()) return child.GetValue().GetInt();
    }
  }

  return defaultValue;
}

double SerializerElement::GetDoubleAttribute(const gd::String& name,
                                             double defaultValue,
                                             gd::String deprecatedName) const {
  if (attributes.find(name) != attributes.end())
    return attributes.find(name)->second.GetDouble();
  else if (!deprecatedName.empty() &&
           attributes.find(deprecatedName) != attributes.end())
    return attributes.find(deprecatedName)->second.GetDouble();
  else {
    if (HasChild(name, deprecatedName)) {
      SerializerElement& child = GetChild(name, 0, deprecatedName);
      if (!child.IsValueUndefined()) return child.GetValue().GetDouble();
    }
  }

  return defaultValue;
}

bool SerializerElement::HasAttribute(const gd::String& name) {
  return attributes.find(name) != attributes.end();
}

SerializerElement& SerializerElement::AddChild(gd::String name) {
  if (!arrayOf.empty()) {
    if (name != arrayOf) {
      std::cout << "WARNING: Adding a child, to a SerializerElement which is "
                   "considered as an array, with a name ("
                << name << ") which is not the same as the array elements ("
                << arrayOf << "). Child was renamed." << std::endl;
      name = arrayOf;
    }
  }

  std::shared_ptr<SerializerElement> newElement(new SerializerElement);
  children.push_back(std::make_pair(name, newElement));

  return *newElement;
}

SerializerElement& SerializerElement::GetChild(std::size_t index) const {
  if (arrayOf.empty()) {
    std::cout << "ERROR: Getting a child from its index whereas the parent is "
                 "not considered as an array."
              << std::endl;
    return nullElement;
  }

  std::size_t currentIndex = 0;
  for (size_t i = 0; i < children.size(); ++i) {
    if (children[i].second == std::shared_ptr<SerializerElement>()) continue;

    if (children[i].first == arrayOf || children[i].first.empty() ||
        (!deprecatedArrayOf.empty() &&
         children[i].first == deprecatedArrayOf)) {
      if (index == currentIndex)
        return *children[i].second;
      else
        currentIndex++;
    }
  }

  std::cout << "ERROR: Request out of bound child at index " << index
            << std::endl;
  return nullElement;
}

SerializerElement& SerializerElement::GetChild(
    gd::String name, std::size_t index, gd::String deprecatedName) const {
  if (!arrayOf.empty()) {
    if (name != arrayOf) {
      std::cout << "WARNING: Getting a child, from a SerializerElement which "
                   "is considered as an array, with a name ("
                << name << ") which is not the same as the array elements ("
                << arrayOf << ")." << std::endl;
      name = arrayOf;
    }
  }

  std::size_t currentIndex = 0;
  for (size_t i = 0; i < children.size(); ++i) {
    if (children[i].second == std::shared_ptr<SerializerElement>()) continue;

    if (children[i].first == name ||
        (!arrayOf.empty() && children[i].first.empty()) ||
        (!deprecatedName.empty() && children[i].first == deprecatedName)) {
      if (index == currentIndex)
        return *children[i].second;
      else
        currentIndex++;
    }
  }

  std::cout << "Child " << name << " not found in SerializerElement::GetChild"
            << std::endl;
  return nullElement;
}

std::size_t SerializerElement::GetChildrenCount(
    gd::String name, gd::String deprecatedName) const {
  if (name.empty()) {
    if (arrayOf.empty()) {
      std::cout
          << "ERROR: Getting children count without specifying name, from a "
             "SerializerElement which is NOT considered as an array."
          << std::endl;
      return 0;
    }

    name = arrayOf;
    deprecatedName = deprecatedArrayOf;
  }

  std::size_t currentIndex = 0;
  for (size_t i = 0; i < children.size(); ++i) {
    if (children[i].second == std::shared_ptr<SerializerElement>()) continue;

    if (children[i].first == name ||
        (!arrayOf.empty() && children[i].first.empty()) ||
        (!deprecatedName.empty() && children[i].first == deprecatedName))
      currentIndex++;
  }

  return currentIndex;
}

bool SerializerElement::HasChild(const gd::String& name,
                                 gd::String deprecatedName) const {
  for (size_t i = 0; i < children.size(); ++i) {
    if (children[i].second == std::shared_ptr<SerializerElement>()) continue;

    if (children[i].first == name ||
        (!deprecatedName.empty() && children[i].first == deprecatedName))
      return true;
  }

  return false;
}

}  // namespace gd
