
#pragma once


   class PtrsVec;    // forward declaration

   //------------------------------------------------------ this can be used to create game object pointers in one string --------------------------------------
   class GamePtrBase {
     public:
        GamePtrBase(int address_, const char* esp_) : address(address_), esp(esp_) {}

        int address;
        const char* esp;

        virtual void init_ptr() = 0;   // ����������� �������, ���-�� �� ����� �������� �� �� ������� GamePtrBase ��� ����������� (�����������)
   };

    // ���� ������� GamePtrBase � ������ �������� �������� ������ � ���, ��� ������ ������� ������ ��������/���������� ���������� ������.
    // �� ����� ������� ������ ������������ �������� ������
    // � � ���� ������ ����������� ���������� <����>, ������� �� ��������� ��� �� �������� (�����������)
    // static std::vector<GamePtrBase*> ptrs;

    template <typename T>
    class GamePtr : public GamePtrBase
    {
      public:
        GamePtr (int address_, const char* esp_) : GamePtrBase(address_, esp_)
        {
            p = nullptr;
            PtrsVec::getVec().push_back(this);
            // ������ ��������� �� ���� (this) ������ ���������� � ������ ������� �������, ����� � �-����. � ������� ����� ����� ��������� �� �� �����
        }

        GamePtr (int address_) : GamePtrBase(address_, "")   // for Skyrim.esm
        {
            p = nullptr;
            PtrsVec::getVec().push_back(this);
        }

        void init_ptr() override
        { 
            if (*esp == 0)  p = RE::TESForm::LookupByID<T>(address);    // for Skyrim.esm
            else            p = RE::TESDataHandler::GetSingleton()->LookupForm<T>(address, esp);    
        }

        T* p;   // ��� ��������� �� ������� ������ ����� �����. �������� ���������� � ��� �����  some_spell.p
    };

    class PtrsVec {
      public:
        static std::vector<GamePtrBase*>& getVec() {
            static std::vector<GamePtrBase*> ptrs;        // ����������� ������, ���������������� ��� ������ ������, ����� ����������� � core.cpp
            return ptrs;
        }
    };
