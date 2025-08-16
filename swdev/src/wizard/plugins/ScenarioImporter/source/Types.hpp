// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
struct StringSlice
{
   const char* begin;
   const char* end;
   StringSlice()
   {
      begin = nullptr;
      end = nullptr;
   }
   StringSlice(const char* aBegin, const char* aEnd) :
      begin(aBegin), end(aEnd)
   {}
};

inline static std::string stdStringFromStringSlice(StringSlice slice)
{
   return std::string(slice.begin, (int)(slice.end - slice.begin));
}
inline static bool isEmpty(StringSlice slice)
{
   return slice.begin == slice.end;
}

inline static size_t length(StringSlice slice)
{
   return size_t(slice.end - slice.begin);
}

static StringSlice stringSliceFromCString(const char* s)
{
   StringSlice slice;
   slice.begin = s;
   while (*s != '\0')
   {
      ++s;
   }
   slice.end = s;
   return slice;
}

static bool equal(StringSlice a, StringSlice b)
{
   size_t lenA = length(a);
   size_t lenB = length(b);
   if (lenA != lenB)
   {
      return false;
   }
   for (size_t i = 0; i < lenA; ++i)
   {
      if (a.begin[i] != b.begin[i])
      {
         return false;
      }
   }
   return true;
}

// TODO get rid of std::map. Maybe make this a vector, maybe some other data structure
using Record = std::map<std::string, StringSlice>;
