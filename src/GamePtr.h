
#pragma once

   class PtrsVec;    // forward declaration

   //------------------------------------------------------ this can be used to create game object pointers in one string --------------------------------------
   class GamePtrBase {
     public:
        GamePtrBase(int address_, const char* esp_) : address(address_), esp(esp_) {}

        int address;
        const char* esp;

        virtual void init_ptr() = 0;   // виртуальная функция, что-бы мы могли вызывать ее из массива GamePtrBase для наследников (полиморфизм)
   };

    // суть массива GamePtrBase и вообще создания базового класса в том, что нельзя создать массив объектов/указателей шаблонного класса.
    // но можно сделать массив нешаблонного базового класса
    // и в него класть наследников различного <типа>, который мы указываем при их создании (полиморфизм)
    // static std::vector<GamePtrBase*> ptrs;

    template <typename T>
    class GamePtr : public GamePtrBase
    {
      public:
        GamePtr (int address_, const char* esp_) : GamePtrBase(address_, esp_)
        {
            p = nullptr;
            PtrsVec::getVec().push_back(this);
            // кладем указатель на этот (this) объект наследника в массив базовых классов, прямо в к-торе. В массиве будет копия указателя но не важно
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

        T* p;   // сам указатель на игровой объект будет здесь. Придется обращаться к ним через  some_spell.p
    };

    class PtrsVec {
      public:
        static std::vector<GamePtrBase*>& getVec() {
            static std::vector<GamePtrBase*> ptrs;        // Статический вектор, инициализируется при первом вызове, будет использован в core.cpp
            return ptrs;
        }
    };
