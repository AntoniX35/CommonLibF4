#include "REL/Module.h"

#include "REX/W32/KERNEL32.h"

namespace REL
{
	Module::Module()
	{
		const auto getFilename = [&]() {
			return REX::W32::GetEnvironmentVariableW(
				ENVIRONMENT.data(),
				_filename.data(),
				static_cast<std::uint32_t>(_filename.size()));
		};

		auto  sz = _filename.size();
		void* handle = nullptr;
		_filename.resize(getFilename());
		if (_filename.empty() || _filename.size() != sz) {
			for (auto& runtime : RUNTIMES) {
				_filename = runtime;
				handle = REX::W32::GetModuleHandleW(_filename.c_str());
				if (handle) {
					break;
				}
			}
		}
		_filePath = _filename;
		if (!handle) {
			stl::report_and_fail(
				std::format(
					"Failed to obtain module handle for: \"{0}\"."
					"You have likely renamed the executable to something unexpected. "
					"Renaming the executable back to \"{0}\" may resolve the issue."sv,
					stl::utf16_to_utf8(_filename).value_or("<unicode conversion error>"s)));
		}

		_base = reinterpret_cast<std::uintptr_t>(handle);
		_natvis = _base;

		load_version();
		load_segments();
	}

	void Module::load_version()
	{
		const auto version = GetFileVersion(_filename);
		if (version) {
			_version = *version;

			constexpr Version maxOG{ 1, 10, 163, 0 };
			constexpr Version minNG{ 1, 10, 980, 0 };
			constexpr Version maxNG{ 1, 10, 984, 0 };
			constexpr Version minAE{ 1, 11, 137, 0 };
			constexpr Version vr{ 1, 2, 72, 0 };

			if (_version == vr) {
				_runtime = Runtime::VR;
			} else if (_version <= maxOG) {
				_runtime = Runtime::OG;
			} else if (_version >= minNG && _version <= maxNG) {
				_runtime = Runtime::NG;
			} else if (_version >= minAE) {
				_runtime = Runtime::AE;
			} else {
				stl::report_and_fail(std::format("Unsupported Fallout 4 runtime: {}", _version));
			}

			return;
		}
		stl::report_and_error(
			std::format(
				"Failed to obtain file version info for: {}\n"
				"Please contact the author of this script extender plugin for further assistance."sv,
				stl::utf16_to_utf8(_filename).value_or("<unicode conversion error>"s)));
	}

	void Module::load_segments()
	{
		const auto dosHeader = reinterpret_cast<const REX::W32::IMAGE_DOS_HEADER*>(_base);
		const auto ntHeader = stl::adjust_pointer<REX::W32::IMAGE_NT_HEADERS64>(dosHeader, dosHeader->lfanew);
		const auto sections = REX::W32::IMAGE_FIRST_SECTION(ntHeader);
		const auto size = std::min<std::size_t>(ntHeader->fileHeader.sectionCount, _segments.size());
		for (std::size_t i = 0; i < size; ++i) {
			const auto& section = sections[i];
			const auto  it = std::find_if(SEGMENTS.begin(), SEGMENTS.end(), [&](auto&& a_elem) {
                constexpr auto size = std::extent_v<decltype(section.name)>;
                const auto     len = std::min(a_elem.size(), size);
                return std::memcmp(a_elem.data(), section.name, len) == 0;
            });
			if (it != SEGMENTS.end()) {
				const auto idx = static_cast<std::size_t>(std::distance(SEGMENTS.begin(), it));
				_segments[idx] = Segment{ _base, _base + section.virtualAddress, section.virtualSize };
			}
		}
	}
}
