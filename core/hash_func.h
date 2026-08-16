#include "includer.h"

class i_block_hasher
{
    public:
    virtual ~i_block_hasher() = default;
    virtual std::string hash(const std::vector<char>& buffer) const = 0;
};

uint32_t calculate_crc32(const std::vector<char>& buffer)
{
    boost::crc_32_type result;
    result.process_bytes(buffer.data(), buffer.size());

    return result.checksum();

}

class crc32_option : public i_block_hasher
{
public:
    std::string hash(const std::vector<char>& buffer) const override
    {
        uint32_t checksum = calculate_crc32(buffer);

        std::ostringstream oss;
        oss << std::hex << std::setw(8) << std::setfill('0') << checksum;
        return oss.str();
    }
};

class md5_option : public i_block_hasher
{
    public:
    
    std::string hash(const std::vector<char>& buffer) const override
    {
        boost::uuids::detail::md5 md5_hasher;
        boost::uuids::detail::md5::digest_type digest{};

        md5_hasher.process_bytes(buffer.data(), buffer.size());
        md5_hasher.get_digest(digest);


        const auto* bytes = reinterpret_cast<const unsigned char*>(&digest);


        std::ostringstream oss;
        auto size_digest = sizeof(digest);
        for(std::size_t index = 0; index < size_digest; ++index)
        {
         oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(bytes[index]);
        }
        return oss.str();
    }
};

std::unique_ptr<i_block_hasher> make_hasher(const std::string& name)
{
    std::string lower = name;

    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {return std::tolower(c);});

    if(lower == "crc32") return std::make_unique<crc32_option>();
    if(lower == "md5") return std::make_unique<md5_option>();

    throw std::invalid_argument("unknown hash algorithm: " + name + " (supported: crc32, md5)");
}