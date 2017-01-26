#pragma once

namespace rage {
	class ScaleformManager
	{
	public:
		struct {
			void* dtor;
			char platform[8];
			char content[8];
		} *info;
		void* memoryWrapper; //0x0008 
		void* memoryWrapper2; //0x0010 
		void* unknown; //0x0018 
		Scaleform::GFx::Loader* gdxLoader; //0x0020
		void* callGameFromFlash; //0x0028
		Scaleform::GFx::FileOpener* fileOpener; //0x0030
		Scaleform::GFx::FSCommandHandler* commandHandler; //0x0038
		Scaleform::GFx::ImageCreator* imageCreator; //0x0040
		void* renderer;//rage::sfRenderer* renderer; //0x0048 
		void* renderConfig;//GFxRenderConfig* renderConfig; //0x0050 
		Scaleform::GFx::Log* gfxlog; //0x0058 
		Scaleform::GFx::DrawTextManager* drawTextManager; //0x0060

		static ScaleformManager* Get()
		{
			return *(ScaleformManager**)((uintptr_t)GetModuleHandle(NULL) + 0x1F3A868);
		}

		static Scaleform::GFx::DrawText* CreateText(const char* putf8Str, const Scaleform::GFx::RectF& viewRect, const Scaleform::GFx::DrawTextManager::TextParams* ptxtParams)
		{
			typedef Scaleform::GFx::DrawText*(*CreateText_)(Scaleform::GFx::DrawTextManager*, const char*, const Scaleform::GFx::RectF&, const Scaleform::GFx::DrawTextManager::TextParams*);
			return CreateText_((uintptr_t)GetModuleHandle(NULL) + 0x15ECB18)(Get()->drawTextManager, putf8Str, viewRect, ptxtParams);
		}
	}; //Size=0x0068
}
