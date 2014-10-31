//
// CXX version of json parser exercise for Leepfrog
// Kent Williams chaircrusher@gmail.com

#include <iostream>
#include <fstream>
#include <sstream>
#include <json/reader.h>
#include <list>

//
// given a string, convert to an XML-escaped string
std::string Escape(const std::string & s)
{
  std::string rval;

  for ( unsigned i = 0; i < s.size(); i++ )
    {
    switch ( s[i] )
      {
      case '&':
        rval += "&amp;";
        break;
      case '<':
        rval += "&lt;";
        break;
      case '>':
        rval += "&gt;";
        break;
      case '"':
        rval += "&quot;";
        break;
      case '\'':
        rval += "&apos;";
        break;
      default:
        rval += s[i];
      }
    }
  return rval;
}

//
// count slashes in a string
unsigned numSlashes(const std::string &s)
{
  unsigned int rval = 0;
  for(unsigned int i = 0; i < s.size(); ++i)
    {
    if(s[i] == '/')
      {
      ++rval;
      }
    }
  return rval;
}

// print indent
void
Indent(std::stringstream &ss,unsigned indent)
{
  for(unsigned int i = 0; i < indent; ++i)
    {
    for(unsigned j = 0; j < 2; ++j)
      {
      ss << " ";
      }
    }
}

class UnorderedList; // forward declaration

//
// c++ descriptor defining link
class Link
{
public:
  Link(const std::string &campus,
             const std::string &section,
             const std::string &name,
             const std::string &url) :
    m_Campus(campus),
    m_Section(section),
    m_Name(name),
    m_Url(url),
    m_List(0)
    {
    }
  ~Link();
  // Access methods
  std::string GetCampus() const { return m_Campus; }
  void SetCampus(const std::string &s) { m_Campus = s; }

  std::string GetSection() const { return m_Section; }
  void SetSection(const std::string &s) { m_Section = s; }

  std::string GetName() const { return m_Name; }
  void SetName(const std::string &s) { m_Name = s; }

  std::string GetUrl() const { return m_Url; }
  void SetUrl(const std::string &s) { m_Url = s; }

  UnorderedList * GetList() { return m_List; }
  void SetList(UnorderedList *l) { m_List = l; }

  // print the HTML for this link
  void XHTML(std::stringstream &ss, unsigned int indent) const;
  // debug print
  void Print()
    {
      std::cerr << m_Campus
                << " " << m_Section
                << " " << m_Name
                << " " << m_Url
                << std::endl;
    }
private:
  std::string     m_Campus;
  std::string     m_Section;
  std::string     m_Name;
  std::string     m_Url;
  UnorderedList * m_List; // sublist, if any
};

//
// class representing the unordered list HTML element
class UnorderedList
{
public:
  ~UnorderedList();
  typedef std::list<Link *> ListType;
  typedef ListType::const_iterator ListIteratorType;
  ListIteratorType begin() const { return m_List.begin(); }
  ListIteratorType end() const { return m_List.end(); }
  Link *front() { return m_List.front(); }
  const Link *front() const { return m_List.front(); }
  void push_back(Link *newMember) { m_List.push_back(newMember); }
  std::string XHTML(std::stringstream &ss,unsigned int indent) const;
private:
  ListType m_List;
};

//
// Destructor
UnorderedList
::~UnorderedList()
{
  //
  // delete all elements of list
  for(ListType::iterator it = m_List.begin();
      it != m_List.end();
      ++it)
    {
    delete (*it);
    }
}

Link
::~Link()
{
  delete m_List;
}

//
// print the XML for a  list
void
Link
::XHTML(std::stringstream &ss,unsigned indent) const
{
  Indent(ss,indent);
  if(this->m_List == 0) // simple li element
    {
    ss << "<li><a href=\""
       << Escape(this->m_Url)
       << "\">"
       << Escape(this->m_Name)
       << "</a></li>" << std::endl;
    }
  else // element with sublist
    {
    ss << "<li class=\"active\"><a href=\""
       << Escape(this->m_Url)
       << "\">"
       << Escape(this->m_Name) //
       << "</a>" << std::endl;
    this->m_List->XHTML(ss,indent + 1);
    Indent(ss,indent);
    ss << "</li>" << std::endl;
    }
  // list element with sublist
}

//
// print the unordered list HTML
std::string
UnorderedList
::XHTML(std::stringstream &ss,unsigned indent) const
{
  Indent(ss,indent);
  ss << "<ul>" << std::endl;
  for(ListIteratorType it = this->begin(); it != this->end(); ++it)
    {
    (*it)->XHTML(ss,indent+1);
    }
  Indent(ss,indent);
  ss << "</ul>" << std::endl;
  return ss.str();
}

//
// this is the main 'assignment' function -- given the supplied JSON
// data input, generate a unordered list with nested lists as needed.
std::string ListHtml(const Json::Value &root, const std::string &campus)
{
  // the top level list
  UnorderedList *topLevelList = new UnorderedList;

  std::string headName = campus;
  headName += " navigation";
  // add top level header list element
  Link *current = new Link(campus,"/",headName,"/");
  topLevelList->push_back(current);

  // keep a stack of the lists
  std::list<UnorderedList *> listStack;
  // start with the top level;
  listStack.push_front(topLevelList);

  Link *last = 0; // need last link, since we don't see that we're
                  // into a sublist until we're processing the link
                  // element one past where the sublist should be
                  // placed.

  // go through the json list.
  for(Json::Value::const_iterator it = root.begin();
      it != root.end(); ++it)
    {
    const Json::Value &curObject = *it;
    // skip the campuses that don't match
    if(curObject["campus"] != campus)
      {
      continue;
      }
    // create a link for the current element
    current = new Link(curObject["campus"].asString(),
                       curObject["section"].asString(),
                       curObject["name"].asString(),
                       curObject["url"].asString());
    if(last != 0)
      {
      // if a slash is added, we're in a sublist
      if(numSlashes(current->GetSection()) > numSlashes(last->GetSection()))
        {
        UnorderedList *newList = new UnorderedList;
        listStack.push_front(newList);
        last->SetList(newList);
        }
      // if less slashes than the last, pop out of the sublist
      else if(numSlashes(current->GetSection()) < numSlashes(last->GetSection()))
        {
        unsigned slashDiff = numSlashes(last->GetSection()) - numSlashes(current->GetSection());
        // if there's a sublist at the end of another sublist, pop out
        // until you reach the containign sublist
        for(unsigned i = 0; i < slashDiff; ++i)
          {
          listStack.pop_front();
          }
        }
      }
    // add this list member to the current list
    listStack.front()->push_back(current);
    // keep track of the last added list member
    last = current;
    }
  // from the top level list, generate the HTML
  std::stringstream ss;
  topLevelList->XHTML(ss,0);
  // deleting the list deletes all the link objects, who will in turn
  // delete their sub-lists, if any.
  delete topLevelList;
  return ss.str();
}

// test fixture
int
main(int argc, char **argv)
{
  if(argc < 3)
    {
    std::cerr << "CxxTest: Usage CxxTest <json-file> <campus>" << std::endl;
    return 1;
    }
  Json::Value root;
  Json::Reader reader;
  std::ifstream f(argv[1]);
  if(!f.is_open())
    {
    std::cerr << "CxxTest: can't open " << argv[1] << std::endl;
    return 1;
    }
  bool parsingSuccessful =  reader.parse(f,root);
  if(!parsingSuccessful)
    {
    std::cerr << "CxxTest: json parsing of " << argv[1]
              << " failed" << std::endl;
    std::cerr << reader.getFormattedErrorMessages() << std::endl;
    return 1;
    }
  if(!root.isArray ())
    {
    std::cerr << "json file needs to define an Array" << std::endl;
    return 1;
    }
  std::string xhtml = ListHtml(root,argv[2]);
  std::cout << xhtml << std::endl;
  return 0;
}
