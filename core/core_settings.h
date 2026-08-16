#include "includer.h"

class filter_byte
{
    private:
    uintmax_t min_size;

    public:

    explicit filter_byte(uintmax_t size = 1) : min_size(size) {}

    uintmax_t get_min_size() const 
    {
        return min_size;
    }

    bool passes(uintmax_t file_size) const
    {
        return file_size >= min_size;
    }
};


class block_settings
{
    private:
    std::size_t block_size;

    public:

    explicit block_settings(std::size_t size = 4096): block_size(size)
    {
        if(block_size == 0)
        {
            throw std::invalid_argument("block size must be greater than 0");
        }
    }
    
        std::size_t get_block_size() const
        {
            return block_size;
        }
    
};