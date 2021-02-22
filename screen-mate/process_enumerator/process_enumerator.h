namespace process_enumerator
{
	extern std::vector<HWND>v_process_enumerator;

	void process_enumerator_thread();

	inline void create_process_enumerator_thread()
	{
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)process_enumerator_thread, NULL, NULL, NULL);
	}
}