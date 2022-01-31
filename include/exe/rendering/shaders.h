#pragma once

namespace GBEmulatorExe 
{
    class Shader
    {
    public:
        Shader(const char* vertexPrg, const char* fragmentPrg);
        ~Shader();

        bool IsValid() const { return m_programId != 0; };
        void UseProgram();
        unsigned GetProgramId() { return m_programId; }

    private:
        void CreateShaders(const char* vertexPrg, const char* fragmentPrg);

        unsigned m_programId = 0;
    };
}