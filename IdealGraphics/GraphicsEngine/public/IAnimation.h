#pragma once

namespace Ideal
{
	class IAnimation
	{
	public:
		IAnimation() {};
		virtual ~IAnimation() {};

	public:
		virtual unsigned int GetFrameCount() abstract;
		virtual float GetFrameRate() abstract;
	};
}