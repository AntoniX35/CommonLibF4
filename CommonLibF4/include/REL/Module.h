#pragma once

#include "REL/Segment.h"
#include "REL/Version.h"

namespace REL
{
	class Module
	{
	public:
		Module(const Module&) = delete;
		Module(Module&&) = delete;

		Module& operator=(const Module&) = delete;
		Module& operator=(Module&&) = delete;

		/**
		 * Identifies a FALLOUT runtime.
		 */
		enum class Runtime : uint8_t
		{
			Unknown = 0,

			/**
			 * The FALLOUT runtime is a pre-Next Generation Update release(version 1.10.163 and earlier).
			 */
			OG = 1 << 1,

			/**
			 * Backwards-compatible alias: F4 is the same as OG.
			 */
			F4 = OG,

			/**
			 * The FALLOUT runtime is a Next-Generation Update release (versions 1.10.980 through 1.10.984).
			 */
			NG = 1 << 2,

			/**
			 * The FALLOUT runtime is an Anniversary Edition release (version 1.11.137 and later).
			 */
			AE = 1 << 3,

			/**
			 * The FALLOUT runtime is Fallout 4 VR.
			 */
			VR = 1 << 4
		};
		[[nodiscard]] static Module& get()
		{
			static Module singleton;
			return singleton;
		}

		[[nodiscard]] constexpr std::uintptr_t base() const noexcept { return _base; }
		[[nodiscard]] stl::zwstring            filename() const noexcept { return _filename; }
		[[nodiscard]] stl::zwstring            filePath() const noexcept { return _filePath; }
		[[nodiscard]] constexpr Segment        segment(Segment::Name a_segment) const noexcept { return _segments[a_segment]; }
		[[nodiscard]] constexpr Version        version() const noexcept { return _version; }

		[[nodiscard]] REX::W32::HMODULE pointer() const noexcept { return reinterpret_cast<REX::W32::HMODULE>(base()); }

		template <class T>
		[[nodiscard]] T* pointer() const noexcept
		{
			return static_cast<T*>(pointer());
		}

		/**
         * Get the type of runtime the currently-loaded FALLOUT module is.
         */
		[[nodiscard]] static FALLOUT_REL Runtime GetRuntime() noexcept
		{
			// If exactly one build-time ENABLE_FALLOUT_* macro is defined, use it as the runtime.
			// Otherwise fall back to the detected runtime at runtime.
#if defined(ENABLE_FALLOUT_OG) && !defined(ENABLE_FALLOUT_NG) && !defined(ENABLE_FALLOUT_AE) && !defined(ENABLE_FALLOUT_VR)
			return Runtime::OG;
#elif defined(ENABLE_FALLOUT_NG) && !defined(ENABLE_FALLOUT_OG) && !defined(ENABLE_FALLOUT_AE) && !defined(ENABLE_FALLOUT_VR)
			return Runtime::NG;
#elif defined(ENABLE_FALLOUT_AE) && !defined(ENABLE_FALLOUT_OG) && !defined(ENABLE_FALLOUT_NG) && !defined(ENABLE_FALLOUT_VR)
			return Runtime::AE;
#elif defined(ENABLE_FALLOUT_VR) && !defined(ENABLE_FALLOUT_OG) && !defined(ENABLE_FALLOUT_NG) && !defined(ENABLE_FALLOUT_AE)
			return Runtime::VR;
#else
			return get()._runtime;
#endif
		}

		/**
		 * Returns whether the current FALLOUT runtime is a post-Nextgen Update Fallout release.
		 */
		[[nodiscard]] static FALLOUT_REL bool IsNG() noexcept
		{
			return GetRuntime() == Runtime::NG;
		}

		/**
         * Returns whether the current FALLOUT runtime is a pre-Nextgen Update Fallout release.
         */
		[[nodiscard]] static FALLOUT_REL bool IsF4() noexcept
		{
			// "F4" (classic Fallout 4 non-VR) is treated as OG
			return GetRuntime() == Runtime::OG;
		}

		/**
         * Returns whether the current FALLOUT runtime is a FALLOUT VR release.
         */
		[[nodiscard]] static FALLOUT_REL_VR bool IsVR() noexcept
		{
#if !defined(ENABLE_FALLOUT_VR)
			return false;
#elif !defined(ENABLE_FALLOUT_AE) && !defined(ENABLE_FALLOUT_NG) && !defined(ENABLE_FALLOUT_OG)
			return true;
#else
			return GetRuntime() == Runtime::VR;
#endif
		}

	private:
		Module();
		~Module() noexcept = default;

		void load_segments();
		void load_version();

		static constexpr auto ENVIRONMENT = L"F4SE_RUNTIME"sv;

		static constexpr std::array<std::wstring_view, 2> RUNTIMES{ { L"Fallout4VR.exe",
			L"Fallout4.exe" } };

		void clear();

		static constexpr std::array SEGMENTS{
			".text"sv,
			".interpr"sv,
			".idata"sv,
			".rdata"sv,
			".data"sv,
			".pdata"sv,
			".tls"sv
		};

		static inline std::uintptr_t _natvis{ 0 };

		std::wstring                        _filename;
		std::wstring                        _filePath;
		std::array<Segment, Segment::total> _segments;
		Version                             _version;
		std::uintptr_t                      _base{ 0 };
		Runtime                             _runtime{ Runtime::NG };
	};
}
