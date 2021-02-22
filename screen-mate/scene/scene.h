
class CScene
{
private:
	enum MOVE_TYPE
	{
		MOVE_NONE = 1 << 0,
		MOVE_LEFT = 1 << 1,
		MOVE_RIGHT = 1 << 2,
		MOVE_SIT = 1 << 3,
		MOVE_SPICIFIC_1 = 1 << 4,
		MOVE_SPECIFIC_2 = 1 << 5,
		MOVE_RUN = 1 << 6,
		MOVE_JUMP = 1 << 7,
	};

	enum FLIP_COND
	{
		NORMAL,
		FLIP_HOR,
		FLIP_VERT,
		FLIP_VERT_HOR,
	};

	enum IMAGES_DATA
	{
		IMAGE_WALK_FRAME_1 = 1 << 1,
		IMAGE_WALK_FRAME_2 = 1 << 2,
		IMAGE_SIT = 1 << 3,
		IMAGE_SPECIFIC_MOVE_1_FRAME_1 = 1 << 4,
		IMAGE_SPECIFIC_MOVE_1_FRAME_2 = 1 << 5,
		IMAGE_SPECIFIC_MOVE_2 = 1 << 6,
		IMAGE_JUMP = 1 << 7,
	};

	class CLogic
	{
	public:
		void Run();
		CScene::FLIP_COND GetCharacterFlip();
		MOVE_TYPE GetCharacterMove();
	private:
		bool IsIntersectTaskBarRect(RECT window_rect, RECT my_window_rect);
		bool IsNextWindowNotBlockPrevWindow(int current_count, std::vector<RECT> v_window_rect);
		bool IsIntersectWindowRect(std::vector<RECT> v_window_rect, RECT& my_window_rect);
		void MoveCharacterToForm(RECT current_window_rect, RECT character_window_rect, float* new_pos);
		void GetAllVisibleWindowRect();
		std::vector<RECT>v_windows_rect;
		RECT current_hovered_window;
		void SetCharacterFlip(CScene::FLIP_COND flip);
		void SelectMove(bool on_ground);
		void __forceinline clear_rect(RECT rect) {
			rect.left = rect.right = rect.top = rect.bottom = 0;
		}

		static const int fall_speed = 2.f;

		bool pressed_lbutton_state;

		float diff_x;

		MOVE_TYPE current_character_move;
		CScene::FLIP_COND current_character_direction;

		float character_move_coord[2];

		bool move_event;

		bool jump_event;

		bool run_event;
		bool is_running;

		int rnd_move;

		DWORD timer;
		DWORD jump_time;

		std::once_flag init_pos_flag;
	};

	CLogic g_Logic;

	class CRenderer
	{
	public:
		void Initialization(LPDIRECT3DDEVICE9 g_pd3dDevice, bool from_memory_or_file)
		{
			static std::once_flag flag;

			std::call_once(flag, [&]() {
				from_memory_or_file ? CreateTexDataFromMemory(g_pd3dDevice) : CreateTexDataFromPath(g_pd3dDevice);
				});
		}
		void CreateTexDataFromMemory(LPDIRECT3DDEVICE9 g_pd3dDevice);
		void CreateTexDataFromPath(LPDIRECT3DDEVICE9 g_pd3dDevice);
		void ClearTexData()
		{
			pmTextureData.clear();
		}
		void ReCreateTexData(LPDIRECT3DDEVICE9 g_pd3dDevice)
		{
			ClearTexData();
			CreateTexDataFromMemory(g_pd3dDevice);
		}
		void AddLine(const ImVec2& from, const ImVec2& to, const ImColor& color, float thickness);
		void AddImage(const ImVec2& position, const ImVec2& size, const ImTextureID pTexture, const ImColor& color, FLIP_COND flip);
		HRESULT LoadTextureFromMemory(const IMAGES_DATA pTexMove, const ImVec2& size, LPDIRECT3DDEVICE9 g_pd3dDevice, BYTE pbImageArrayTarget[], const int sizeof_arr);
		HRESULT LoadTextureFromPath(const IMAGES_DATA pTexMove, const ImVec2& size, LPDIRECT3DDEVICE9 g_pd3dDevice, std::string path);
		IMAGES_DATA GetAnimation(CScene::MOVE_TYPE moveType);
		std::map<CScene::IMAGES_DATA, LPDIRECT3DTEXTURE9>pmTextureData;	
	private:
		DWORD timer = 0;
		bool texture_next = false;
		bool count_texture = false;
	};

	CRenderer g_Render;
public:
	bool use_image_in_memory;
	void Run();
};
extern std::unique_ptr<CScene>m_pScene;