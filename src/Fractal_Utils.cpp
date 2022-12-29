#include "Fractal_Utils.h"

std::string prefPath = SDL_GetPrefPath("MakeFractals", "MakeFractals");

std::map<const char*, indexMapAlgorithm> indexMapAlgorithms{
	{"Top to bottom", &topToBottom},
	{"Left to right", &leftToRight},
	{"Outwards from middle (horizontal)", &outwardsFromMiddleH},
	{"Outwards from middle (vertical)", &outwardsFromMiddleV},
	//{"Spiral", &spiral},
};

void HelpMarker(const char* desc) {
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

bool sdlColorEdit3(const char* label, SDL_Color* col, ImGuiColorEditFlags flags) {

	float colFloats[3] = { (float)col->r / 255, (float)col->g / 255 , (float)col->b / 255 };

	bool result = ImGui::ColorEdit3(label, colFloats, flags);

	*col = { (unsigned char)(colFloats[0] * 255), (unsigned char)(colFloats[1] * 255) , (unsigned char)(colFloats[2] * 255) };

	return result;

}

void topToBottom(int i, int* x, int* y, int width, int height) {

	*x = i % width;
	*y = (height - 1) - i / width;

}

void leftToRight(int i, int* x, int* y, int width, int height) {

	*x = i / height;
	*y = i % height;

}

void outwardsFromMiddleH(int i, int* x, int* y, int width, int height) {

	int _x = i / height;
	if (_x % 2 == width % 2) {
		*x = (width + _x) / 2;
	}
	else {
		*x = (width - _x) / 2;
	}

	*y = i % height;

}

void outwardsFromMiddleV(int i, int* x, int* y, int width, int height) {

	int _y = i / width;
	if (_y % 2 == height % 2) {
		*y = (height + _y) / 2;
	}
	else {
		*y = (height - _y) / 2;
	}

	*x = i % width;

}


/*
Taken from https://stackoverflow.com/questions/65815332/flipping-a-surface-vertically-in-sdl2
 */
void SDL_FlipSurface(SDL_Surface* surface) {

	SDL_LockSurface(surface);

	int pitch = surface->pitch; // row size
	char* temp = new char[pitch]; // intermediate buffer
	char* pixels = (char*)surface->pixels;

	for (int i = 0; i < surface->h / 2; ++i) {
		// get pointers to the two rows to swap
		char* row1 = pixels + i * pitch;
		char* row2 = pixels + (surface->h - i - 1) * pitch;

		// swap rows
		memcpy(temp, row1, pitch);
		memcpy(row1, row2, pitch);
		memcpy(row2, temp, pitch);
	}

	delete[] temp;

	SDL_UnlockSurface(surface);

}