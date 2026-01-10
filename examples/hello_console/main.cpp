#include <modulus/core/log.hpp>

int main() {
	MOD_INFO("Welcome to Modulus Engine version {}", 0.1);
	MOD_WARN("This is a warning!");
	MOD_ERROR("Something went wrong with code {}", 404);

	return 0;
}