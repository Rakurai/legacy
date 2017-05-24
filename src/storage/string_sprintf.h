template<class T>
inline auto to_c(T&& arg) -> decltype(std::forward<T>(arg)) {
    return std::forward<T>(arg);
}

inline char const* to_c(std::string const& s) { return s.c_str(); }
inline char const* to_c(std::string& s) { return s.c_str(); }

template<class... Args>
int my_printf(char const* fmt, Args&&... args) {
    return std::printf(fmt, to_c(args)...);
}
