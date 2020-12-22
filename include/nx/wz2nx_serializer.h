#include <boost/noncopyable.hpp>
#include <boost/container/vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/streambuf.hpp>
#include <unordered_map>
#include <string>
#include <cstdint>
#include "wzreader.h"

namespace wz {
namespace nx {
class WZ2NXSerializer : boost::noncopyable {
   public:
    WZ2NXSerializer(){};

   public:
    void Parse(const std::string& path_to_wz, const std::string& path_to_nx);
private:
    void WriteNodeLevel(boost::container::vector<wz::WZNode*>& node_levels, std::fstream &bw);
    void WriteUOL(wz::WZNode* node, std::fstream& bw);
    void WriteNode(wz::WZNode* node, std::fstream& bw, uint32_t next_child_id);
    void WriteString(const std::string& value, std::fstream& bw);
    void WriteMP3(wz::WZNode* node, std::fstream& bw);
    void WriteBitmap(wz::WZNode* node, std::fstream& bw);
    void EnsureMultiple(int32_t multiple, std::fstream& bw);
    uint32_t AddString(const std::string& value);
    void Clear();
private:
    std::unordered_map<wz::WZNode*, uint32_t> _nodes;
    boost::container::vector<wz::WZNode*> _sounds_nodes;
    boost::container::vector<wz::WZNode*> _bitmaps_nodes;
    boost::container::vector<std::string> _strings;
    std::unordered_map<uint32_t, wz::WZNode*> _uol_nodes;
    boost::asio::streambuf _nodes_buffer;
    boost::asio::streambuf _strings_buffer;
    boost::asio::streambuf _bitmaps_buffer;
    boost::asio::streambuf _audios_buffer;
};
}  // namespace nx
}  // namespace wz
