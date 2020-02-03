#include "JAJF.h"
#include <windows.h>

namespace JAJF
{
    bool JSONObject::bThrowErrors = true;

    JSONObject::JSONObject() : value_string(""), value_int(0), value_double(0.0), value_bool(false), bit_type(types_object) {}
    JSONObject::JSONObject(const JSONObject& obj) noexcept : value_string(obj.value_string), value_int(obj.value_int), value_double(obj.value_double), value_bool(obj.value_bool), bit_type(obj.bit_type)
    {
        objects.clear();
        objects = obj.objects;
    }
    void JSONObject::operator=(const JSONObject& obj)
    {
        value_string = obj.value_string;
        value_int = obj.value_int;
        value_double = obj.value_double;
        value_bool = obj.value_bool;
        bit_type = obj.bit_type;
        objects.clear();
        objects = obj.objects;
    }
    bool JSONObject::Exists(const std::string& key) const
    {
        return bit_type == types_object && (objects.find(key) != objects.end());
    }
    bool JSONObject::Exists(int index) const
    {
        return bit_type == types_array && (objects.find(std::to_string(index)) != objects.end());
    }
    JSONObject& JSONObject::operator[](const std::string& key)
    {
        return Get(key);
    }
    JSONObject& JSONObject::Get(const std::string& key)
    {
        if (bit_type == types_array)
        {
            objects.clear();
            bit_type = types_object;
        }

        if (Exists(key))
            return objects[key];
        objects[key];
        return objects[key];
    }
    JSONObject& JSONObject::operator[](int index)
    {
        return Get(index);

    }
    JSONObject& JSONObject::Get(int index)
    {
        if (bit_type == types_object)
        {
            objects.clear();
            bit_type = types_array;
        }
        std::string num = std::to_string(index);
        if (Exists(num))
            return objects[num];

        if ((int)objects.size() < index)
        {
            for (int i = (int)objects.size(); i < index; ++i)
                objects[std::to_string(i)];
        }
        objects[num];
        return objects[num];

    }
    JSONObject JSONObject::Remove(const std::string& key)
    {
        if (bit_type == types_object && Exists(key))
        {
            JSONObject obj = Get(key);
            objects.erase(key);
            return obj;
        }
        return JSONObject();
    }
    JSONObject JSONObject::Remove(int index)
    {
        if (bit_type == types_array && Exists(index))
        {
            JSONObject obj = Get(index);
            objects.erase(std::to_string(index));
            return obj;
        }
        return JSONObject();
    }
    void JSONObject::operator=(const std::string& s)
    {
        if (bit_type != types_string)
            EraseData();
        bit_type = types_string;
        value_string = s;
    }
    void JSONObject::operator=(const int i)
    {
        if (bit_type != types_int)
            EraseData();
        bit_type = types_int;
        value_int = i;
        value_double = (double)i;
        value_bool = (i != 0);
    }
    void JSONObject::operator=(const double d)
    {
        if (bit_type != types_double)
            EraseData();
        bit_type = types_double;
        value_double = d;
        value_int = (int)d;
        value_bool = (d != 0);
    }
    void JSONObject::operator=(const bool b)
    {
        if (bit_type != types_bool)
            EraseData();
        bit_type = types_bool;
        value_bool = b;
        value_double = b ? 1.0 : 0.0;
        value_int = b ? 1 : 0;
    }
    void JSONObject::operator=(const char* c)
    {
        *this = std::string(c);
    }

    template<> std::string JSONObject::Value() const
    {
        return value_string;
    }
    template<> int JSONObject::Value() const
    {
        return value_int;
    }
    template<> double JSONObject::Value() const
    {
        return value_double;
    }
    template<> float JSONObject::Value() const
    {
        return (float)value_double;
    }
    template<> bool JSONObject::Value() const
    {
        return value_bool;
    }

    JSONObject JSONObject::ParseArray(const char* data, long length, int& x, int& y, const char* file, const char** newPointer) const
    {
        const char* pointer = data;
        JSONObject object;
        bool requireComma = false;
        int index = 0;
        while (*pointer != ']')
        {
            if (requireComma && *pointer != ',')
                ThrowError("JAJF PARSE ERROR", (std::string("Missing comma at:\nLine: ") + std::to_string(y) + " Col: " + std::to_string(x)).c_str(), file);
            else if (requireComma && *pointer == ',')
            {
                requireComma = false;
                ++pointer;
                ++x;
                continue;
            }
            const std::string number = GetNumber(pointer, x, y);
            if (number.length() > 0)
            {
                //We have a number!
                if (number.find(".") != std::string::npos)
                    //We have a decimal
                    object[index] = std::stod(number);
                else
                    //We have an int
                    object[index] = std::stoi(number);
                ++index;
            }
            else
            {
                //We have a non-number
                if (*pointer == '{')
                {
                    //We have an object
                    ++pointer;
                    ++x;
                    object[index] = Parse(pointer, length, x, y, file, &pointer);
                    ++index;
                }
                else if (*pointer == '[')
                {
                    //We have an array
                    ++pointer;
                    ++x;
                    object[index] = ParseArray(pointer, length, x, y, file, &pointer);
                    ++pointer;
                    ++x;
                    ++index;
                }
                else if (*pointer == '"')
                {
                    //We have a string
                    object[index] = GetString(++pointer, ++x, y);
                    ++index;
                }
            }
            if (*pointer == ',')
            {
                ++pointer;
                ++x;
            }
            SkipWhitespace(pointer, x, y);
        }
        if (newPointer)
            *newPointer = pointer;
        return object;
    }

    JSONObject JSONObject::Parse(const char* data, long length, int& x, int& y, const char* file, const char** newPointer) const
    {
        auto distance = [&](const char* buffer, const char* pointer) -> signed long
        {
            return (signed long)(buffer - pointer);
        };

        JSONObject object;
        bool requireComma = false;
        for (const char* pointer = data + (*data == '{' ? 1 : 0); distance(data + length, pointer) > 0; ++pointer)
        {
            ++x;
            SkipWhitespace(pointer, x, y);
            if (distance(data + length, pointer) <= 0)
                break;

            switch (*pointer)
            {
            case '"':
            {
                if (requireComma)
                    ThrowError("JAJF PARSE ERROR", ((std::string("Unexpected character!\nExpected ',' got '") + *pointer + "'\nAt Line: ") + std::to_string(y) + " Col: " + std::to_string(x)).c_str());

                //Key or string
                std::string keyName = GetString(++pointer, ++x, y);
                if (object.Exists(keyName))
                    ThrowError("JAJF PARSE ERROR", (std::string("Redefinition of variable!\nLine: ") + std::to_string(y) + " Col: " + std::to_string(x)).c_str());

                SkipWhitespace(pointer, x, y);
                if (*pointer == ':')
                {
                    ++pointer;
                    ++x;
                    SkipWhitespace(pointer, x, y);
                    const std::string number = GetNumber(pointer, x, y);
                    if (number.length() > 0)
                    {
                        //We have a number!
                        if (number.find(".") != std::string::npos)
                            //We have a decimal
                            object[keyName] = std::stod(number);
                        else
                            //We have an int
                            object[keyName] = std::stoi(number);
                    }
                    else
                    {
                        //We have a non-number
                        if (*pointer == '{')
                        {
                            //We have an object
                            ++pointer;
                            ++x;
                            object[keyName] = Parse(pointer, length, x, y, file, &pointer);
                        }
                        else if (*pointer == '[')
                        {
                            //We have an array
                            ++pointer;
                            ++x;
                            object[keyName] = ParseArray(pointer, length, x, y, file, &pointer);
                            ++pointer;
                            ++x;
                        }
                        else if (*pointer == '"')
                        {
                            //We have a string
                            object[keyName] = GetString(++pointer, ++x, y);
                        }
                    }
                    --pointer;
                    --x;
                    requireComma = true;
                }
                else
                {
                    ThrowError("JAJF PARSE ERROR", ((std::string("Unexpected character!\nExpected ':' got '") + *pointer + "'\nAt Line: ") + std::to_string(y) + " Col: " + std::to_string(x)).c_str(), file);
                }
                break;
            }
            case ',':
                if (requireComma)
                    requireComma = false;
                else
                    ThrowError("JAJF PARSE ERROR", ((std::string("Unexpected character!\nExpected Value, got ','\nAt Line: ") + std::to_string(y) + " Col: " + std::to_string(x)).c_str()), file);

                break;

            case '}':
                ++pointer;
                ++x;
                if (newPointer)
                    * newPointer = pointer;
                return object;
                break;
            default:
                if (*pointer > 32)
                    ThrowError("JAJF PARSE ERROR", ((std::string("Unexpected character '") + *pointer + "'!\nAt Line: ") + std::to_string(y) + " Col: " + std::to_string(x)).c_str(), file);

            }
        }
        ThrowError("JAJF PARSE ERROR", "Unexpected end of line!", file);
        return object;
    }

    const std::string JSONObject::GetString(const char*& start, int& x, int& y) const
    {
        std::string name;
        while (*start != '"')
        {
            name += *start;
            ++start;
            ++x;
        }
        ++start;
        ++x;

        return name;
    }

    const std::string JSONObject::GetNumber(const char*& start, int& x, int& y, const char* fileName) const
    {
        std::string number;
        if (*start == '-')
        {
            number += "-";
            ++start;
            ++x;
        }
        bool eNotationUsed = false;
        bool eSignUsed = false;
        bool decimalUsed = false;
        while ((*start >= '0' && *start <= '9') || *start == '.' || *start == 'e' || *start == 'E' || *start == '-' || *start == '+')
        {
            if ((*start == 'e' || *start == 'E') && eNotationUsed)
                ThrowError("JAJF PARSE ERROR!", ((std::string("Unexpected character!\nOnly one instance of 'e' or 'E' permitted.\nAt Line: ") + std::to_string(y) + " Col: " + std::to_string(x)).c_str()), fileName);
            else if ((*start == 'e' || *start == 'E') && !eNotationUsed)
                eNotationUsed = true;

            if ((*start == '-' || *start == '+') && (!eNotationUsed || eSignUsed))
                ThrowError("JAJF PARSE ERROR", ((std::string("Unexpected character!\nAt Line: ") + std::to_string(y) + " Col: " + std::to_string(x)).c_str()), fileName);
            else if ((*start == '-' || *start == '+') && (eNotationUsed && !eSignUsed))
                eSignUsed = true;

            if (*start == '.' && decimalUsed)
                ThrowError("JAJF PARSE ERROR", ((std::string("Unexpected character!\nExpected 0-9 got '.'\nAt Line: ") + std::to_string(y) + " Col: " + std::to_string(x)).c_str()), fileName);

            else if (*start == '.' && !decimalUsed)
                decimalUsed = true;

            number += *(start++);
            ++x;
        }
        return number;
    }

    void JSONObject::EraseData()
    {
        value_string = "";
        value_int = 0;
        value_double = 0.0;
        value_bool = false;
        bit_type = 0b00000;
        objects.clear();
    }

    void JSONObject::SkipWhitespace(const char*& pointer, int& x, int& y) const
    {
        while (*pointer <= 32)
        {
            if (*pointer == '\n')
            {
                ++y;
                x = 1;
            }
            else
                ++x;

            ++pointer;
        }
    }

    void JSONObject::ThrowError(const char* title, const char* errorMSG, const char* fileName) const
    {
        if (!bThrowErrors)
            return;

        bool result = false;
        if (fileName)
            result = MessageBox(NULL, (std::string("File: ") + fileName + "\n" + errorMSG).c_str(), title, MB_OKCANCEL | MB_ICONERROR) == IDCANCEL;
        else
            result = MessageBox(NULL, errorMSG, title, MB_OKCANCEL | MB_ICONERROR) == IDCANCEL;

        if (result)
        {
            exit(1);
        }
    }

    bool JSONObject::ReadFromFile(const char* path)
    {
        FILE* file = nullptr;
        fopen_s(&file, path, "r");
        if (file)
        {
            fseek(file, 0, SEEK_END);
            long length = ftell(file);
            rewind(file);
            const char* buffer = new char[length + 1]{ '\0' };
            fread_s((void*)buffer, length, length, 1, file);
            fclose(file);
            int x = 1, y = 1;
            if (buffer[0] == '[')
                *this = ParseArray(buffer+1, length, x, y, path);
            else
                *this = Parse(buffer, length, x, y, path);
            delete[] buffer;
            return true;
        }
        else
        {
            ThrowError("JAJF READ ERROR", "Error reading file!", path);
        }
        return false;
    }

    const std::string JSONObject::Stringify(int& tabbing, bool isArray) const
    {
        std::string str;
        if (isArray)
            str = std::string(tabbing, '\t') + "[\n";
        else
            str = std::string(tabbing, '\t') + "{\n";
        ++tabbing;
        bool start = true;
        for (auto pair : objects)
        {
            if (start)
                start = false;
            else
                str += ",\n";

            str += std::string(tabbing, '\t');

            if (!isArray)
                str += "\"" + pair.first + "\": ";

            switch (pair.second.bit_type)
            {
            case types_bool:
                str += pair.second.value_bool ? "true" : "false";
                break;
            case types_string:
                str += "\"" + pair.second.value_string + "\"";
                break;
            case types_int:
                str += std::to_string(pair.second.value_int);
                break;
            case types_double:
                str += std::to_string(pair.second.value_double);
                break;
            case types_object:
                str += "\n" + pair.second.Stringify(tabbing, false);
                break;
            case types_array:
                str += "\n" + pair.second.Stringify(tabbing, true);
                break;
            default:
                ThrowError("JAJF STRINGIFY ERROR!", (std::string("Unknown type error!\nKey: ") + pair.first).c_str());
                break;
            }
        }
        --tabbing;
        if (isArray)
            str += "\n" + std::string(tabbing, '\t') + "]";
        else
            str += "\n" + std::string(tabbing, '\t') + "}";
        return str;
    }

    void JSONObject::WriteToFile(const char* path) const
    {
        int tabbing = 0;
        const std::string str = Stringify(tabbing);

        FILE* file;
        fopen_s(&file, path, "wb");
        if (file)
        {
            char* buffer = (char*)str.c_str();
            fwrite(buffer, str.length(), 1, file);
            fclose(file);
        }
        else
        {
            ThrowError("JAJF WRITE ERROR", "Error writing to file!", path);
        }
    }

    void JSONObject::SetThrowErrors(bool bThrowErrors)
    {
        JSONObject::bThrowErrors = bThrowErrors;
    }

    int JSONObject::GetArraySize() const
    {
        if (bit_type == types_array)
            return (int)objects.size();
        else
            return 0;
    }

    JSONObject::iterator JSONObject::begin()
    {
        return iterator(objects.begin());
    }

    JSONObject::iterator JSONObject::end()
    {
        return iterator(objects.end());
    }

    //ITERATOR
    JSONObject::iterator::iterator(std::map<std::string, JSONObject>::iterator it) : it(it) {}
    JSONObject::iterator::iterator(const iterator& it) : it(it.it) {}
    JSONObject::iterator& JSONObject::iterator::operator=(const JSONObject::iterator& it)
    {
        this->it = it.it;
        return *this;
    }
    JSONObject::iterator& JSONObject::iterator::operator++()
    {
        ++it;
        return *this;
    }
    JSONObject::iterator& JSONObject::iterator::operator--()
    {
        --it;
        return *this;
    }
    JSONObject::iterator JSONObject::iterator::operator++(int)
    {
        iterator _it = *this;
        ++it;
        return _it;
    }
    JSONObject::iterator JSONObject::iterator::operator--(int)
    {
        iterator _it = *this;
        --it;
        return _it;
    }
    bool JSONObject::iterator::operator==(const iterator& it)
    {
        return it.it == this->it;
    }
    bool JSONObject::iterator::operator!=(const iterator& it)
    {
        return it.it != this->it;
    }
    JSONObject& JSONObject::iterator::operator*()
    {
        return it->second;
    }
    JSONObject* JSONObject::iterator::operator->()
    {
        return &it->second;
    }

}