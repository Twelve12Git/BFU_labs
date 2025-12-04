#include <iostream>

// лаба 8 вариант 1
struct Season {
    using month_t = const char*;
    month_t monthes[3];
    const char* name;
    Season* next_season = nullptr;

    void swap_month_with_next(int n){
        month_t tmp = monthes[n];
        monthes[n] = next_season->monthes[n];
        next_season->monthes[n] = tmp;
    }
};

struct MagicCalendar {
    Season* entry_season = nullptr;
    
    ~MagicCalendar() {
        clear();
    }
    
    MagicCalendar* add_season(const Season& s) {
        Season* new_season = new Season(s);
        
        if (entry_season == nullptr) {
            entry_season = new_season;
            entry_season->next_season = entry_season;
        } else {
            new_season->next_season = entry_season->next_season;
            entry_season->next_season = new_season;
            entry_season = new_season;
        }
        return this;
    }
    
    void clear() {
        if (entry_season == nullptr) return;
        
        Season* current = entry_season;
        Season* to_delete;
        
        do {
            to_delete = current;
            current = current->next_season;
            delete to_delete;
        } while (current != entry_season);
        
        entry_season = nullptr;
    }
    
    void print() const {
        if (entry_season == nullptr) {
            std::cout << "Calendar is empty\n";
            return;
        }
        
        Season* current = entry_season;
        
        do {
            std::cout << "Season " <<  current->name << ": ";
            for (int i = 0; i < 3; ++i) {
                if (current->monthes[i] != nullptr) {
                    std::cout << current->monthes[i] << " ";
                }
            }
            std::cout << "\n";
            current = current->next_season;
        } while (current != entry_season);
    }
};

int main(){
    MagicCalendar calendar;
    calendar
        .add_season(Season{.monthes={"Сентябрь", "Хренабрь", "Фигабрь"}, .name="Осень"})
        ->add_season(Season{.monthes={"Сентябрь1", "Хренабрь1", "Фигабрь1"}, .name="Осень1"})
        ->add_season(Season{.monthes={"Сентябрь2", "Хренабрь2", "Фигабрь2"}, .name="Осень2"})
        ->add_season(Season{.monthes={"Сентябрь3", "Хренабрь3", "Фигабрь3"}, .name="Осень3"})
        ->print()
    ;
    std::cout<<std::endl;
    calendar.entry_season->swap_month_with_next(2);
    calendar.print();
    return 0;
}
