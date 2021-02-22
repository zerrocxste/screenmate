#include "../includes.h"

#include "../images/images.h"

std::unique_ptr<CScene>m_pScene;

void CScene::Run()
{
	this->g_Render.Initialization(DXWFGetD3DDevice(), this->use_image_in_memory);

	this->g_Logic.Run();

	auto characterDirection = this->g_Logic.GetCharacterFlip();

	auto characterMove = this->g_Logic.GetCharacterMove();

	ImVec2 wndsize = ImGui::GetWindowSize();

	this->g_Render.AddImage(
		ImVec2(5.f, 1.f), 
		ImVec2(wndsize.x - 10.f, wndsize.y), 
		this->g_Render.pmTextureData[this->g_Render.GetAnimation(characterMove)],
		ImColor(255, 255, 255), 
		characterDirection
	);
}

bool CScene::CLogic::IsIntersectTaskBarRect(RECT window_rect, RECT my_window_rect)
{
	float y_frame_pos_other_window = window_rect.bottom;
	float y_pos_my_window = my_window_rect.bottom;

	if (y_pos_my_window >= y_frame_pos_other_window)
		return true;

	return false;
}

bool CScene::CLogic::IsNextWindowNotBlockPrevWindow(int current_count, std::vector<RECT> v_window_rect)
{
	for (int i = current_count - 1; i >= 1; i--)
	{
		const auto &current_rect = v_window_rect[current_count];
		const auto &my_rect = v_window_rect[0];
		const auto &rect = v_window_rect[i];

		auto intersect_y = rect.top <= current_rect.top && rect.bottom >= current_rect.top;
		auto intersect_x_right = rect.right >= (current_rect.left + (my_rect.left - current_rect.left));
		auto intersect_x_left = rect.left <= (current_rect.right - (current_rect.right - my_rect.right));

		if (intersect_y && intersect_x_right && intersect_x_left)
		{
			return false;
		}
	}

	return true;
}

bool CScene::CLogic::IsIntersectWindowRect(std::vector<RECT> v_window_rect, RECT& my_window_rect)
{
	for (int i = 0; i < v_window_rect.size(); i++)
	{
		float left_frame_pos_other_window = v_window_rect[i].left;
		float left_pos_my_window = my_window_rect.left;

		float right_frame_pos_other_window = v_window_rect[i].right;
		float right_pos_my_window = my_window_rect.right;

		float y_frame_pos_other_window = v_window_rect[i].top;

		float y_pos_my_window = my_window_rect.bottom;
		float y_pos_other_window = v_window_rect[i].bottom;

		if (left_pos_my_window < left_frame_pos_other_window - 50.f)
			continue;

		if (right_pos_my_window > right_frame_pos_other_window + 50.f)
			continue;

		if (y_pos_other_window <= 0 || y_frame_pos_other_window <= 0)
			continue;

		if ((y_frame_pos_other_window - y_pos_my_window) <= -30)
			continue;

		if (y_pos_my_window >= y_frame_pos_other_window && 
			this->IsNextWindowNotBlockPrevWindow(i, v_window_rect))
		{
			this->current_hovered_window = v_window_rect[i];
			return true;
		}
		else
		{
			this->clear_rect(this->current_hovered_window);
		}
	}

	return false;
}

void CScene::CLogic::MoveCharacterToForm(RECT current_window_rect, RECT character_window_rect, float* new_pos)
{
	bool lbutton_pressed = GetAsyncKeyState(VK_LBUTTON);

	if (lbutton_pressed && this->pressed_lbutton_state == false)
	{
		this->diff_x = character_window_rect.left - current_window_rect.left;
		this->pressed_lbutton_state = true;
	}
	else if (lbutton_pressed == false)
		this->pressed_lbutton_state = false;

	if (lbutton_pressed)
		new_pos[0] = current_window_rect.left + this->diff_x;

	new_pos[1] = current_window_rect.top - (character_window_rect.bottom - character_window_rect.top);
}

void CScene::CLogic::GetAllVisibleWindowRect()
{
	for (auto hwnd : process_enumerator::v_process_enumerator)
	{
		RECT window_rect;
		if (GetWindowRect(hwnd, &window_rect))
		{
			this->v_windows_rect.push_back(window_rect);
		}	
	}
}

void CScene::CLogic::Run()
{
	RECT rect_taskbar; //taskbar rect
	GetWindowRect(FindWindow("Shell_traywnd", NULL), &rect_taskbar);

	RECT rect_screen; //desktop rect
	GetWindowRect(GetDesktopWindow(), &rect_screen);
	rect_screen.bottom -= (rect_taskbar.bottom - rect_taskbar.top);

	RECT my_rect; //character window rect
	GetWindowRect(DXWFGetHWND(), &my_rect);

	this->GetAllVisibleWindowRect();

	//RECT rect_current_window; //current active window rect
	//GetWindowRect(GetForegroundWindow(), &rect_current_window);

	std::call_once(this->init_pos_flag, [&]() {
		this->character_move_coord[0] = rect_screen.right / 2.f, this->character_move_coord[1] = rect_screen.bottom / 2.f;
		});

	this->character_move_coord[0] = my_rect.left;
	this->character_move_coord[1] = my_rect.top;

	bool on_ground = false;

	bool isIntersectWindowRect = this->IsIntersectWindowRect(this->v_windows_rect, my_rect);

	bool isIntersectTaskBarRect = this->IsIntersectTaskBarRect(rect_screen, my_rect);

	if (isIntersectWindowRect)
	{
		this->MoveCharacterToForm(this->current_hovered_window, my_rect, this->character_move_coord);
		on_ground = true;
	}
	else if (isIntersectTaskBarRect)
	{
		on_ground = true;
	}

	this->SelectMove(on_ground);

	auto current_move = this->GetCharacterMove();

	auto character_speed = (current_move & CScene::MOVE_TYPE::MOVE_RUN) ? 4.f : 2.f;

	if (current_move & CScene::MOVE_TYPE::MOVE_LEFT)
	{
		this->character_move_coord[0] -= character_speed;
		this->SetCharacterFlip(CScene::FLIP_COND::NORMAL);
	}
	else if (current_move & CScene::MOVE_TYPE::MOVE_RIGHT)
	{
		this->character_move_coord[0] += character_speed;
		this->SetCharacterFlip(CScene::FLIP_COND::FLIP_HOR);
	}

	if (current_move & CScene::MOVE_TYPE::MOVE_JUMP)
	{
		this->character_move_coord[1] -= 5.f;
	}
	else if (isIntersectWindowRect == false && isIntersectTaskBarRect == false)
	{
		this->character_move_coord[1] += fall_speed;
		this->pressed_lbutton_state = false;
	}

	if (my_rect.right < 0.f || my_rect.left > rect_screen.right)
	{
		this->character_move_coord[0] = rect_screen.right / 2.f - 100.f;
		this->character_move_coord[1] = 1.f;
	}

	this->v_windows_rect.clear();

	if (g_bIsMouseCaptured)
		return;

	MoveWindow(DXWFGetHWND(), this->character_move_coord[0], this->character_move_coord[1], my_rect.right - my_rect.left, my_rect.bottom - my_rect.top, TRUE);
}

CScene::FLIP_COND CScene::CLogic::GetCharacterFlip()
{
	return CScene::FLIP_COND(this->current_character_direction);
}

CScene::MOVE_TYPE CScene::CLogic::GetCharacterMove()
{
	return MOVE_TYPE(this->current_character_move);
}

void CScene::CLogic::SetCharacterFlip(CScene::FLIP_COND flip)
{
	this->current_character_direction = flip;
}

void CScene::CLogic::SelectMove(bool on_ground)
{
	DWORD move = 0;

	int rnd_timeout = (rand() + 500) * 2; //15

	if (this->move_event == false && on_ground)
	{
		this->rnd_move |= 1 << rand() % 6;
		this->timer = GetTickCount();
		this->move_event = true;
	}
	else if (this->move_event && GetTickCount() - this->timer < rnd_timeout)
	{
		move |= rnd_move;
	}
	else
	{
		this->move_event = false;
		this->rnd_move = 0;
	}

	if (move & CScene::MOVE_TYPE::MOVE_LEFT || move & CScene::MOVE_TYPE::MOVE_RIGHT)
	{
		if (this->run_event == false)
		{
			if (rand() % 10 == 0)
			{	
				this->is_running = true;
			}
			this->run_event = true;
		}	
	}
	else
	{
		this->run_event = false;
		this->is_running = false;
	}

	if (this->is_running)
	{
		move |= CScene::MOVE_TYPE::MOVE_RUN;
	}

	if (this->jump_event == false &&
		(rand() % 2000) == CScene::MOVE_TYPE::MOVE_JUMP &&
		on_ground)
	{
		this->jump_event = true;
		this->jump_time = GetTickCount();
	}
	else if (this->jump_event && GetTickCount() - this->jump_time < 400)
	{
		move |= CScene::MOVE_TYPE::MOVE_JUMP;
	}
	else
	{
		this->jump_event = false;
	}

	this->current_character_move = (MOVE_TYPE)move;
}

void CScene::CRenderer::AddLine(const ImVec2& from, const ImVec2& to, const ImColor& color, float thickness)
{
	auto window = ImGui::GetCurrentWindow();

	window->DrawList->AddLine(from, to, ImGui::ColorConvertFloat4ToU32(color), thickness);
}

void CScene::CRenderer::AddImage(const ImVec2& position, const ImVec2& size, const ImTextureID pTexture, const ImColor& color, FLIP_COND flip)
{
	auto window = ImGui::GetCurrentWindow();

	ImRect bb(position, ImVec2(position.x + size.x, position.y + size.y));

	ImVec2 minUV{ 0.f, 0.f };
	ImVec2 maxUV{ 1.f, 1.f };

	if (flip == FLIP_HOR)
	{
		minUV.x = 1.f;
		maxUV.x = 0.f;
	}
	else if (flip == FLIP_VERT)
	{
		minUV.y = 1.f;
		maxUV.y = 0.f;
	}
	else if (flip == FLIP_VERT_HOR)
	{
		minUV.x = 1.f;
		maxUV.x = 0.f;
		minUV.y = 1.f;
		maxUV.y = 0.f;
	}

	window->DrawList->AddImage(pTexture, bb.Min, bb.Max, minUV, maxUV, ImGui::ColorConvertFloat4ToU32(color));
}

HRESULT CScene::CRenderer::LoadTextureFromMemory(const IMAGES_DATA pTexMove,
	const ImVec2& size, LPDIRECT3DDEVICE9 g_pd3dDevice, BYTE pbImageArrayTarget[], const int sizeof_arr)
{
	if (pmTextureData[pTexMove] == nullptr)
	{
		HRESULT hrImageLoadRes =
			D3DXCreateTextureFromFileInMemoryEx(g_pd3dDevice, pbImageArrayTarget, sizeof_arr,
				size.x, size.y, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, NULL, NULL, NULL, &pmTextureData[pTexMove]);

		if (hrImageLoadRes != NULL)
		{
			console::attach("error");
			std::cout << __FUNCTION__ << " -> Image Load for " << pTexMove << " return " << hrImageLoadRes << std::endl;
		}
		return hrImageLoadRes;
	}
}

HRESULT CScene::CRenderer::LoadTextureFromPath(const IMAGES_DATA pTexMove,
	const ImVec2& size, LPDIRECT3DDEVICE9 g_pd3dDevice, std::string path)
{
	if (pmTextureData[pTexMove] == nullptr)
	{
		HRESULT hrImageLoadRes =
			D3DXCreateTextureFromFileExA(g_pd3dDevice, path.c_str(),
				size.x, size.y, D3DX_DEFAULT, NULL, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, NULL, NULL, NULL, &pmTextureData[pTexMove]);

		if (hrImageLoadRes != NULL)
		{
			console::attach("error");
			std::cout << __FUNCTION__ << " -> Image Load for " << pTexMove << " return " << hrImageLoadRes << std::endl;
		}
		return hrImageLoadRes;
	}
}

void CScene::CRenderer::CreateTexDataFromPath(LPDIRECT3DDEVICE9 g_pd3dDevice)
{
	std::string current_path = _getcwd(NULL, NULL) + std::string("\\");

	HRESULT hLoadingIsOK = NULL;

	hLoadingIsOK = LoadTextureFromPath(IMAGES_DATA::IMAGE_WALK_FRAME_1, ImVec2(200.f, 200.f), g_pd3dDevice, std::string(current_path + "image_walk_frame_1.png").c_str());
	hLoadingIsOK = LoadTextureFromPath(IMAGES_DATA::IMAGE_WALK_FRAME_2, ImVec2(200.f, 200.f), g_pd3dDevice, std::string(current_path + "image_walk_frame_2.png").c_str());
	hLoadingIsOK = LoadTextureFromPath(IMAGES_DATA::IMAGE_SIT, ImVec2(200.f, 200.f), g_pd3dDevice, std::string(current_path + "image_sit.png").c_str());
	hLoadingIsOK = LoadTextureFromPath(IMAGES_DATA::IMAGE_SPECIFIC_MOVE_1_FRAME_1, ImVec2(200.f, 200.f), g_pd3dDevice, std::string(current_path + "image_specific_move_1_frame_1.png").c_str());
	hLoadingIsOK = LoadTextureFromPath(IMAGES_DATA::IMAGE_SPECIFIC_MOVE_1_FRAME_2, ImVec2(200.f, 200.f), g_pd3dDevice, std::string(current_path + "image_specific_move_1_frame_2.png").c_str());
	hLoadingIsOK = LoadTextureFromPath(IMAGES_DATA::IMAGE_SPECIFIC_MOVE_2, ImVec2(200.f, 200.f), g_pd3dDevice, std::string(current_path + "image_specific_move_2.png").c_str());
	hLoadingIsOK = LoadTextureFromPath(IMAGES_DATA::IMAGE_JUMP, ImVec2(200.f, 200.f), g_pd3dDevice, std::string(current_path + "image_jump.png").c_str());

	if (hLoadingIsOK != NULL)
	{
		std::cout << "\nImages are not loaded, they may be broken or missing. The names for your images should represent the following names:\n\n"
			<< "image_walk_frame_1.png\n"
			<< "image_walk_frame_2.png\n"
			<< "image_sit.png\n"
			<< "image_specific_move_1_frame_1.png\n"
			<< "image_specific_move_1_frame_2.png\n"
			<< "image_specific_move_2.png\n"
			<< "image_jump.png\n";
	}
}

void CScene::CRenderer::CreateTexDataFromMemory(LPDIRECT3DDEVICE9 g_pd3dDevice)
{
	LoadTextureFromMemory(IMAGES_DATA::IMAGE_WALK_FRAME_1, ImVec2(200.f, 200.f), g_pd3dDevice, cat_move1, sizeof(cat_move1));
	LoadTextureFromMemory(IMAGES_DATA::IMAGE_WALK_FRAME_2, ImVec2(200.f, 200.f), g_pd3dDevice, cat_move2, sizeof(cat_move2));
	LoadTextureFromMemory(IMAGES_DATA::IMAGE_SIT, ImVec2(200.f, 200.f), g_pd3dDevice, cat_sit, sizeof(cat_sit));
	LoadTextureFromMemory(IMAGES_DATA::IMAGE_SPECIFIC_MOVE_1_FRAME_1, ImVec2(200.f, 200.f), g_pd3dDevice, cat_lick_balls1, sizeof(cat_lick_balls1));
	LoadTextureFromMemory(IMAGES_DATA::IMAGE_SPECIFIC_MOVE_1_FRAME_2, ImVec2(200.f, 200.f), g_pd3dDevice, cat_lick_balls2, sizeof(cat_lick_balls2));
	LoadTextureFromMemory(IMAGES_DATA::IMAGE_SPECIFIC_MOVE_2, ImVec2(200.f, 200.f), g_pd3dDevice, cat_meow, sizeof(cat_meow));
	LoadTextureFromMemory(IMAGES_DATA::IMAGE_JUMP, ImVec2(200.f, 200.f), g_pd3dDevice, cat_jump, sizeof(cat_jump));
}

CScene::IMAGES_DATA CScene::CRenderer::GetAnimation(CScene::MOVE_TYPE moveType)
{
	auto move_change_time = !(moveType & CScene::MOVE_TYPE::MOVE_RUN) ? 700 : 300;

	if (count_texture == false && GetTickCount() - timer > move_change_time)
	{
		texture_next = !texture_next;
		count_texture = true;
	}
	else if (count_texture)
	{
		timer = GetTickCount();
		count_texture = false;
	}

	if (moveType & CScene::MOVE_TYPE::MOVE_NONE)
	{
		return IMAGES_DATA::IMAGE_WALK_FRAME_2;
	}
	else if (moveType & CScene::MOVE_TYPE::MOVE_JUMP)
	{
		return IMAGES_DATA::IMAGE_JUMP;
	}
	else if (moveType & CScene::MOVE_TYPE::MOVE_LEFT || moveType & CScene::MOVE_TYPE::MOVE_RIGHT)
	{
		auto tex = texture_next == false ? IMAGES_DATA::IMAGE_WALK_FRAME_2 : IMAGES_DATA::IMAGE_WALK_FRAME_1;
		return tex;
	}
	else if (moveType & CScene::MOVE_TYPE::MOVE_SIT)
	{
		return IMAGES_DATA::IMAGE_SIT;
	}
	else if (moveType & CScene::MOVE_TYPE::MOVE_SPICIFIC_1)
	{
		auto tex = texture_next == false ? IMAGES_DATA::IMAGE_SPECIFIC_MOVE_1_FRAME_2 : IMAGES_DATA::IMAGE_SPECIFIC_MOVE_1_FRAME_1;
		return tex;
	}
	else if (moveType & CScene::MOVE_TYPE::MOVE_SPECIFIC_2)
	{
		return IMAGES_DATA::IMAGE_SPECIFIC_MOVE_2;
	}
	
	return IMAGES_DATA::IMAGE_WALK_FRAME_2;
}