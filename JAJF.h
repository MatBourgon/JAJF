#pragma once
//Just Another JSON Format
#include <map>
#include <string>
namespace JAJF
{
    class JSONObject
    {
    protected:

        //Whether or not to throw errors
        static bool bThrowErrors;

        //List of entries
        std::map<std::string, JSONObject> objects;

        //Values

        std::string value_string;               //00001
        int value_int;                          //00010
        double value_double;                    //00100
        bool value_bool;                        //01000

        //Object type
        unsigned char bit_type;

        //Erases all data
        void EraseData();

        //Parse an object, returns a JSONObject
        JSONObject Parse(const char* data, long length, int& x, int& y, const char* file, const char** newPointer = nullptr) const;

        //Parse an array, returns a JSONObject (array type)
        JSONObject ParseArray(const char* data, long length, int& x, int& y, const char* file, const char** newPointer = nullptr) const;

        //Get a string by parsing
        const std::string GetString(const char*& start, int& x, int& y) const;

        //Get a string by parsing
        const std::string GetNumber(const char*& start, int& x, int& y, const char* fileName = nullptr) const;

        //Ignores and skips past whitespace characters
        void SkipWhitespace(const char*& pointer, int& x, int& y) const;

        //Throws out errors
        void ThrowError(const char* title, const char* errorMSG, const char* fileName = nullptr) const;

        //Converts the object tree into a formatted string
        const std::string Stringify(int& tabbing, bool isArray = false) const;

    public:

        //Types of data
        enum types_
        {
            types_string = 0b000001,
            types_int = 0b000010,
            types_double = 0b000100,
            types_bool = 0b001000,
            types_array = 0b010000,
            types_object = 0b100000,
        };

        //Get object type (compare with JAJF::types_::*
        unsigned char GetType() { return bit_type; };

        //Create an empty JSON Object
        JSONObject();

        //Get a copy of a JSON object
        JSONObject(const JSONObject& obj) noexcept;

        //Get a copy of a JSON object
        void operator=(const JSONObject& obj);

        //Check if an entry exists
        bool Exists(const std::string& key) const;

        //Get an object's entry by key
        JSONObject& operator[](const std::string& key);

        //Get an array's entry by index
        JSONObject& operator[](int index);

        //Set the object to a string
        void operator=(const std::string& s);

        //Set the object to an int
        void operator=(const int i);

        //Set object to a double
        void operator=(const double d);

        //Set object to a bool
        void operator=(const bool b);

        //Set object to a string
        void operator=(const char* c);

        template<typename T, typename ...Ts>
        void AddToArray(T value, Ts...list)
        {
            if (bit_type != types_array)
            {
                EraseData();
                bit_type = types_array;
            }
            operator[](GetArraySize()) = value;
            AddToArray(list...);
        }

        //Set object to an array
        template<typename T>
        void AddToArray(T value)
        {
            if (bit_type != types_array)
            {
                EraseData();
                bit_type = types_array;
            }
            operator[](GetArraySize()) = value;
        }

        //Get the value stored in an object
        //Valid types:
        //int, double, bool, std::string
        template<typename T>
        T Value() const;

        //Read JSON from a file on disk. Returns true if read was successful, false otherwise.
        bool ReadFromFile(const char* path);

        //Write JSON to file on disk.
        void WriteToFile(const char* path) const;

        //Enables/Disables error throwing
        void static SetThrowErrors(bool bThrowError);

        //Get size of array (if its an array)
        int GetArraySize() const;

    };
};
