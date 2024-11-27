#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <cstdint>
#include <bitset>
#include <array>
#include <iomanip>

constexpr uint64_t PAGE_SIZE = 4096;           // Page size (4 KB)
constexpr uint64_t ENTRIES_PER_TABLE = 512;    // 512 entries per table
constexpr uint64_t PAGE_OFFSET_MASK = 0xFFF;   // Mask for offset (12 bits)
constexpr uint64_t PAGE_INDEX_MASK = 0x1FF;    // Mask for index (9 bits)
constexpr uint64_t DEFAULT_FLAGS = 0b11;       // Present (bit 0) and RW (bit 1)

constexpr int PT_SHIFT = 12;
constexpr int PD_SHIFT = 21;
constexpr int PDPT_SHIFT = 30;
constexpr int PML4_SHIFT = 39;

// Function to extract the index at each level
uint64_t get_index(uint64_t address, int shift) {
	return (address >> shift) & PAGE_INDEX_MASK;
}

// Structure for a page table entry
struct PageTableEntry {
	uint64_t value = 0;

	PageTableEntry(uint64_t addr = 0, uint64_t flags = 0) {
		value = (addr & ~PAGE_OFFSET_MASK) | (flags & PAGE_OFFSET_MASK);
	}

	uint64_t get_address() const {
		return value & ~PAGE_OFFSET_MASK;
	}

	uint64_t get_flags() const {
		return value & PAGE_OFFSET_MASK;
	}
};

// Class for a page table
class PageTable {
public:
	PageTableEntry entries[ENTRIES_PER_TABLE] = { 0 };
};

// Structure to keep track of allocated tables
struct AllocatedTable {
	uint64_t address;
	PageTable* table;
	int level; // 0: PML4, 1: PDPT, 2: PD, 3: PT
};

// Page table manager
class PageTableManager {
public:
	PageTableManager(uint64_t base_addr, bool verbose_mode)
		: base_address(base_addr), current_address(base_addr + PAGE_SIZE), verbose(verbose_mode) {
		PML4 = new PageTable();
		allocated_tables.push_back({ base_address, PML4, 0 });
		table_map[base_address] = PML4;
	}

	~PageTableManager() {
		for (auto& alloc : allocated_tables) {
			delete alloc.table;
		}
	}

	void map_pages(uint64_t physical_address, uint64_t virtual_address, uint64_t num_pages, uint64_t flags) {
		for (uint64_t i = 0; i < num_pages; ++i) {
			uint64_t va = virtual_address + i * PAGE_SIZE;
			uint64_t pa = physical_address + i * PAGE_SIZE;
			map_page(va, pa, flags);
		}
	}

	void write_tables_to_file(const std::string& filename, bool silent_mode) {
		std::ofstream outfile(filename, std::ios::binary);
		if (!outfile) {
			std::cerr << "Error opening file: " << filename << "\n";
			return;
		}

		const char* level_names[] = { "PML4", "PDPT", "PD", "PT" };

		for (const auto& alloc : allocated_tables) {
			const PageTable* table = alloc.table;
			uint64_t buffer[ENTRIES_PER_TABLE] = { 0 };

			if (verbose) {
				std::cout << level_names[alloc.level] << " Table at address 0x" << std::hex << alloc.address << ":\n";
			}

			for (size_t i = 0; i < ENTRIES_PER_TABLE; ++i) {
				const PageTableEntry& entry = table->entries[i];
				if (entry.value != 0) {
					buffer[i] = entry.value;
					if (verbose) {
						uint64_t addr = entry.get_address();
						uint64_t flags = entry.get_flags();
						if (alloc.level == 3) { // PT level
							std::cout << "  [" << std::hex << i << "] -> 0x" << addr << ", flags = 0b" << std::bitset<12>(flags) << "\n";
						}
						else {
							std::cout << "  [" << std::hex << i << "] -> 0x" << addr << "\n";
						}
					}
				}
			}

			outfile.write(reinterpret_cast<const char*>(buffer), PAGE_SIZE);

			if (verbose) {
				std::cout << "\n";
			}
		}

		if (!silent_mode) {
			std::cout << "Page tables written to file " << filename << "\n";
			std::cout << "Total pages written: " << std::dec << allocated_tables.size() << "\n";
		}
	}

private:
	uint64_t base_address;
	uint64_t current_address;
	PageTable* PML4;
	std::vector<AllocatedTable> allocated_tables;
	std::unordered_map<uint64_t, PageTable*> table_map;
	bool verbose;

	void map_page(uint64_t virtual_address, uint64_t physical_address, uint64_t flags) {
		uint64_t pml4_index = get_index(virtual_address, PML4_SHIFT);
		uint64_t pdpt_index = get_index(virtual_address, PDPT_SHIFT);
		uint64_t pd_index = get_index(virtual_address, PD_SHIFT);
		uint64_t pt_index = get_index(virtual_address, PT_SHIFT);

		PageTableEntry& pml4_entry = PML4->entries[pml4_index];
		PageTable* pdpt_table = nullptr;

		// PML4 level
		if (pml4_entry.value == 0) {
			pdpt_table = new PageTable();
			uint64_t addr = allocate_address();
			pml4_entry = PageTableEntry(addr, DEFAULT_FLAGS);
			allocated_tables.push_back({ addr, pdpt_table, 1 });
			table_map[addr] = pdpt_table;
		}
		else {
			uint64_t addr = pml4_entry.get_address();
			pdpt_table = table_map[addr];
		}

		// PDPT level
		PageTableEntry& pdpt_entry = pdpt_table->entries[pdpt_index];
		PageTable* pd_table = nullptr;

		if (pdpt_entry.value == 0) {
			pd_table = new PageTable();
			uint64_t addr = allocate_address();
			pdpt_entry = PageTableEntry(addr, DEFAULT_FLAGS);
			allocated_tables.push_back({ addr, pd_table, 2 });
			table_map[addr] = pd_table;
		}
		else {
			uint64_t addr = pdpt_entry.get_address();
			pd_table = table_map[addr];
		}

		// PD level
		PageTableEntry& pd_entry = pd_table->entries[pd_index];
		PageTable* pt_table = nullptr;

		if (pd_entry.value == 0) {
			pt_table = new PageTable();
			uint64_t addr = allocate_address();
			pd_entry = PageTableEntry(addr, DEFAULT_FLAGS);
			allocated_tables.push_back({ addr, pt_table, 3 });
			table_map[addr] = pt_table;
		}
		else {
			uint64_t addr = pd_entry.get_address();
			pt_table = table_map[addr];
		}

		// PT level
		pt_table->entries[pt_index] = PageTableEntry(physical_address, flags);
	}

	uint64_t allocate_address() {
		uint64_t addr = current_address;
		current_address += PAGE_SIZE;
		return addr;
	}
};

// Function to parse a number from string (supports 0x and 0b prefixes)
uint64_t parse_number(const std::string& str) {
	if (str.rfind("0x", 0) == 0) {
		return std::stoull(str, nullptr, 16);
	} else if (str.rfind("0b", 0) == 0) {
		return std::stoull(str.substr(2), nullptr, 2);
	} else {
		return std::stoull(str);
	}
}

// Function to strip comments from a line
std::string strip_comments(const std::string& line) {
	auto pos = line.find('#');
	if (pos != std::string::npos) {
		return line.substr(0, pos);
	}
	return line;
}

// Function to display help
void print_help(const std::string& program_name) {
	std::cout << "Usage: " << program_name << " <input_file> --base=<address> [-v|-s] -o <output_file>\n";
	std::cout << "\nOptions:\n";
	std::cout << "  <input_file>        File containing physical and virtual addresses\n";
	std::cout << "  --base=<address>    Base address for page tables (hexadecimal or binary)\n";
	std::cout << "  -v                  Enable verbose mode to display details\n";
	std::cout << "  -s                  Enable silent mode for no output\n";
	std::cout << "  -o <output_file>    Name of the output file\n";
	std::cout << "  --help              Display this help message\n";
	std::cout << "\nInput file format:\n";
	std::cout << "  physical_address virtual_address number_of_pages flags\n";
	std::cout << "  # Comments start with '#' and are ignored\n";
	std::cout << "\nExample:\n";
	std::cout << "  0x7000 0xFFFFFFFF7FFFD000 1 0x3   # Map 1 page with flags 0x3\n";
}

// Main function
int main(int argc, char* argv[]) {
	if (argc < 2) {
		print_help(argv[0]);
		return EXIT_FAILURE;
	}

	bool verbose = false;
	bool silent = false;
	std::string base_arg;
	std::string output_file;
	std::string input_file;

	// Parsing arguments
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "--help") {
			print_help(argv[0]);
			return EXIT_SUCCESS;
		}
		else if (arg.rfind("--base=", 0) == 0) {
			base_arg = arg.substr(7);
		}
		else if (arg == "-v") {
			if (silent) {
				std::cerr << "Options -v and -s cannot be used together\n";
				return EXIT_FAILURE;
			}
			verbose = true;
		}
		else if (arg == "-s") {
			if (verbose) {
				std::cerr << "Options -v and -s cannot be used together\n";
				return EXIT_FAILURE;
			}
			silent = true;
		}
		else if (arg == "-o") {
			if (i + 1 < argc) {
				output_file = argv[++i];
			} else {
				std::cerr << "Option -o requires an argument\n";
				return EXIT_FAILURE;
			}
		}
		else if (arg[0] == '-') {
			std::cerr << "Unknown option: " << arg << "\n";
			return EXIT_FAILURE;
		}
		else {
			input_file = arg;
		}
	}

	if (input_file.empty()) {
		std::cerr << "Missing input file\n";
		return EXIT_FAILURE;
	}

	if (base_arg.empty()) {
		std::cerr << "Missing or invalid --base argument\n";
		return EXIT_FAILURE;
	}

	if (output_file.empty()) {
		std::cerr << "Missing -o argument (output file)\n";
		return EXIT_FAILURE;
	}

	uint64_t base_address = parse_number(base_arg);

	PageTableManager pt_manager(base_address, verbose);

	// Reading the input file
	std::ifstream infile(input_file);
	if (!infile) {
		std::cerr << "Error opening file: " << input_file << "\n";
		return EXIT_FAILURE;
	}

	std::string line;
	while (std::getline(infile, line)) {
		line = strip_comments(line);
		std::istringstream iss(line);
		std::string physical_address_str, virtual_address_str, num_pages_str, flags_str;

		if (!(iss >> physical_address_str >> virtual_address_str >> num_pages_str >> flags_str)) {
			continue; // Ignore empty or malformed lines
		}

		uint64_t physical_address = parse_number(physical_address_str);
		uint64_t virtual_address = parse_number(virtual_address_str);
		uint64_t num_pages = parse_number(num_pages_str);
		uint64_t flags = parse_number(flags_str);

		pt_manager.map_pages(physical_address, virtual_address, num_pages, flags);
	}

	pt_manager.write_tables_to_file(output_file, silent);

	return EXIT_SUCCESS;
}
