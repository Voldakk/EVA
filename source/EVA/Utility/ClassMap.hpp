#pragma once

/*
 * Concatenate preprocessor tokens A and B without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define PPCAT_NX(A, B) A##B

// Macro for registering a class. Should be put inside the class declaration
#define REGISTER_CLASS(MAPNAME, TYPE, NAME)                                                                                                \
    static inline PPCAT_NX(EVA::ClassRegister, MAPNAME)<TYPE> s_Register = PPCAT_NX(EVA::ClassRegister, MAPNAME)<TYPE>(std::string(NAME)); \
                                                                                                                                           \
  public:                                                                                                                                  \
    std::string GetTypeId() const override { return s_Register.typeId; }                                                                   \
                                                                                                                                           \
  private:


#define CREATE_CLASS_MAP(MAPNAME, TYPE)                                                                                                    \
    namespace EVA                                                                                                                          \
    {                                                                                                                                      \
        class PPCAT_NX(ClassMap, MAPNAME)                                                                                                  \
        {                                                                                                                                  \
          public:                                                                                                                          \
            typedef std::map<std::string, std::shared_ptr<TYPE> (*)()> map_type;                                                           \
            inline static map_type* map;                                                                                                   \
                                                                                                                                           \
            static map_type* GetMap()                                                                                                      \
            {                                                                                                                              \
                if (!map) { map = new map_type(); }                                                                                        \
                return map;                                                                                                                \
            }                                                                                                                              \
                                                                                                                                           \
            static std::shared_ptr<TYPE> Create(const std::string& id)                                                                     \
            {                                                                                                                              \
                const auto it = GetMap()->find(id);                                                                                        \
                                                                                                                                           \
                if (it == GetMap()->end()) return nullptr;                                                                                 \
                                                                                                                                           \
                return it->second();                                                                                                       \
            }                                                                                                                              \
                                                                                                                                           \
            template<typename T>                                                                                                           \
            static std::shared_ptr<TYPE> CreateT()                                                                                         \
            {                                                                                                                              \
                return std::make_shared<T>();                                                                                              \
            }                                                                                                                              \
                                                                                                                                           \
            static std::vector<std::string> GetIds()                                                                                       \
            {                                                                                                                              \
                std::vector<std::string> ids;                                                                                              \
                                                                                                                                           \
                for (map_type::const_iterator it = map->begin(); it != map->end(); ++it)                                                   \
                {                                                                                                                          \
                    ids.push_back(it->first);                                                                                              \
                }                                                                                                                          \
                                                                                                                                           \
                return ids;                                                                                                                \
            }                                                                                                                              \
        };                                                                                                                                 \
                                                                                                                                           \
        template<typename T>                                                                                                               \
        struct PPCAT_NX(ClassRegister, MAPNAME)                                                                                            \
        {                                                                                                                                  \
            std::string typeId;                                                                                                            \
            PPCAT_NX(ClassRegister, MAPNAME)(std::string s) : typeId(s)                                                                    \
            {                                                                                                                              \
                if (PPCAT_NX(ClassMap, MAPNAME)::GetMap()->find(s) == PPCAT_NX(ClassMap, MAPNAME)::GetMap()->end())                        \
                {                                                                                                                          \
                    PPCAT_NX(ClassMap, MAPNAME)::GetMap()->insert(std::make_pair(s, &PPCAT_NX(ClassMap, MAPNAME)::CreateT<T>));            \
                }                                                                                                                          \
            }                                                                                                                              \
        };                                                                                                                                 \
    }
