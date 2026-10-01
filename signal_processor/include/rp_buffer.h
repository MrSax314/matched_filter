
#include <memory>

namespace matched_filter {

class RPBuffer {
  public:
	RPBuffer() = default;
	~RPBuffer() = default;
	std::shared_ptr<RPBuffer> GetNextAvailable(const int& buffer_size);

  private:
	int temp;
};

}  // namespace matched_filter