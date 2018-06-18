#pragma once

namespace ke {
	class INoCopy
	{
	public:
		INoCopy() {
		
		};
		virtual ~INoCopy() {
		
		};

		//No Copy
		INoCopy(const INoCopy&) = delete;
		INoCopy& operator=(const INoCopy&) = delete;

	private:

	};

	
}
