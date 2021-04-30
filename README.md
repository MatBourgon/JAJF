## JAJF - Just another JSON Format

### This was created more-so as practice, but I've come to use this in my own projects. Thus, I've released it and plan on touching it up whenever I notice any bugs.

### Quick setup:

1. Add JAJF.h and JAJF.cpp to your project
2. That's it!

JAJF uses the JAJF namespace (JAJF::JSONObject). For best efficiency and use, make sure to use References if holding objects. For example:

    JSONObject root;
    root.ReadFromFile("file.json");
    JSONObject& player = root["player"];
    printf("Player name: %s", player["name"].Value<std::string>().c_str());
    
Using storage variables aren't required, you can do everything in a single line, other than reading:

    JSONObject root;
    root.ReadFromFile("file.json");
    printf("Player name: %s", root["player"]["name"].Value<std::string>().c_str());
    
### Error Throwing

By default, JAJF comes with error throwing by default. If an invalid character is found in the file, it will throw an error that will let the user either quit or continue running the program (as well and the position it occured).
To disable this feature, add this line of code before use (only required once): 

    JAJF::SetThrowErrors(false);
    
Passing in true will result in the re-enabling of error throwing.
