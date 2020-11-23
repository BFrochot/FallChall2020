#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

// Stratégie points quand l'adversaire va terminer

// Lancer sorts gratuits avant rest si  -ne bloque pas notre potion
//                                      -adversaire ne pourra finir potion avant ou en même temps

// Accumuler
// Acheter si adversaire peut acheter ?

int objective = -1;
char brewed = 0;
int visited[10][10][10][10][40];

// Potions
typedef struct pot_t {
    int cost[4];
    int id;
    int price;
    float value;
    int path[30];
    bool got_path;
    struct pot_t *next;
    struct pot_t *prev;
} pot_t;

// Spells
typedef struct spell_t {
    int cost[4];
    int id;
    char sid;
    bool usable;
    bool repeatable;
    struct spell_t *next;
    struct spell_t *prev;
} spell_t;

// Spells I can learn
typedef struct learnable_t {
    int cost[4];
    int id;
    int taxe;
    int sum;
    float sum_value;
    bool repeatable;
    int index;
    struct learnable_t *next;
    struct learnable_t *prev;
} learnable_t;

// Bfs queue
typedef struct queue {
    char inv[4];
    char path[30];
    bool spell[20];
    char sum;
    struct queue *next;
} queue;

// set done pot
void pot_done(queue *q, pot_t *pot) {
    char i = 0;

    pot->got_path = true;
    while (q->path[i]) {
        fprintf(stderr,"%d ", q->path[i]);
        pot->path[i] = q->path[i];
        i++;
    }
    pot->path[i] = q->path[i];
    pot->value = (float)q->sum / pot->price;
    fprintf(stderr,"  Val %.4f Price %d Id %d Op %.0f\n", pot->value, pot->price, pot->id, (float)q->sum);
}

// take a queue element and list of pots
// returns the numbers of unbrewed pots that can be brewed
char can_do_pot(queue *q, pot_t *pot) {
    bool can_do_pot;
    char a = 0;
    while (pot != NULL) {
        if (pot->got_path) {
            pot = pot->next;
            continue;
        }
        can_do_pot = true;
        for (int i = 0; i < 4; i++) {
            if (q->inv[i] + pot->cost[i] < 0) {
                can_do_pot = false;
                break;
            }
        }
        if (can_do_pot) {
            pot_done(q, pot);
            ++a;
        }
        pot = pot->next;
    }
    return a;
}

// true if the _spell is launchable _multiple times from _queue state
bool launchable(spell_t *sp, queue *q, char multiple) {
    if (multiple > 1 && !(sp->repeatable))
        return false;
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        if (q->inv[i] + multiple * sp->cost[i] < 0)
            return false;
        sum += q->inv[i] + multiple * sp->cost[i];
    }
    return sum < 11;
}

// true if the _state of spells is new
bool diff_state(queue *q, int sum, int states[40], char multiple, spell_t *spells) {
    bool b;
    int i;

    // calculate sum
    // if resting else
    if (sum == 1) {
        sum = 0;
        while (spells != NULL){
            sum += spells->id;
            spells = spells->next;
        }
    } else {
        while (spells != NULL){
            if (q->spell[spells->sid])
                sum += spells->id;
            spells = spells->next;
        }
    }

    // find sum
    i = 1;
    while (states[i] && i < 38) {
        if (sum == states[i])
            return false;
        i++;
    }

    // add new state
    while(states[i] && i < 38)
        i++;
    states[i] = sum;
    states[i + 1] = 0;
    return true;
}

// graveyard for queue mallocs
// if q == NULL ask for memory, else put in graveyard
queue *graveyard(queue *q) {
    static queue *zombies = NULL;

    if (q == NULL) {
        if (zombies == NULL)
            return malloc(sizeof(queue));
        q = zombies;
        zombies = zombies->next;
        return q;
    }
    q->next = zombies;
    zombies = q;
    return NULL;
}

void set_visited(char *inv) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                for (int l = 0; l < 10; l++) {
                    visited[i][j][k][l][0] = 100;
                    for (int m = 1; m < 40; m++) {
                        visited[i][j][k][l][m] = 0;
                    }
                }
            }
        }
    }
    visited[inv[0]][inv[1]][inv[2]][inv[3]][0] = 1;

}

// algorythm setting pots best paths
void bfs(char *inv, spell_t *first_spell, char nb_pots, pot_t *first_pot) {
    int cmp_pots = 0;
    char multiple;
    queue *q;
    queue *new;
    int a;
    int save;
    spell_t *sp;

    clock_t timer = clock(); 
    set_visited(inv);

    queue *last_q;
    queue *first_q;
    first_q = graveyard(NULL);
    first_q->inv[0] = inv[0];
    first_q->inv[1] = inv[1];
    first_q->inv[2] = inv[2];
    first_q->inv[3] = inv[3];
    first_q->path[0] = 0;
    first_q->sum = 0;
    first_q->next = NULL;
    sp = first_spell;
    while (sp != NULL) {
        first_q->spell[sp->sid] = sp->usable;
        sp = sp->next;
    }
    last_q = first_q;

    int cmp = 0;
    while (first_q != NULL){
        cmp++;
        q = first_q;
        first_q = first_q->next;
        if (q == last_q)
            last_q = NULL;

        cmp_pots += can_do_pot(q, first_pot);
        if (cmp_pots == nb_pots) {
            fprintf(stderr, "cmp = %d\n", cmp);
            clock_t time_spent = clock() - timer;
            fprintf(stderr, "time spent %ldns average %lfns\n", time_spent, (double)time_spent / cmp); 
            //free all list
            graveyard(q);
            while(first_q != NULL) {
                q = first_q;
                first_q = first_q->next;
                graveyard(q);
            }
            return;
        }

        sp = first_spell;
        while (sp != NULL) {
            multiple = 0;
            while (++multiple) {
                //if usable
                //if launchable (got cost to launch)
                //if never got this inventory state with the same number of spells
                //if never got this spells state
                if (q->spell[sp->sid] && launchable(sp, q, multiple)
                && visited[q->inv[0] + multiple * sp->cost[0]][q->inv[1] + multiple * sp->cost[1]][q->inv[2] + multiple * sp->cost[2]][q->inv[3] + multiple * sp->cost[3]][0] > q->sum
                && diff_state(q, -sp->id, visited[q->inv[0] + multiple * sp->cost[0]][q->inv[1] + multiple * sp->cost[1]][q->inv[2] + multiple * sp->cost[2]][q->inv[3] + multiple * sp->cost[3]], multiple, first_spell))
                {
                    new = graveyard(NULL);
                    new->next = NULL;
                    if (last_q != NULL)
                        last_q->next = new;
                    else
                        first_q = new;
                    last_q = new;

                    new->inv[0] = q->inv[0] + multiple * sp->cost[0];
                    new->inv[1] = q->inv[1] + multiple * sp->cost[1];
                    new->inv[2] = q->inv[2] + multiple * sp->cost[2];
                    new->inv[3] = q->inv[3] + multiple * sp->cost[3];
                    new->sum = q->sum + 1;
                    visited[q->inv[0] + sp->cost[0]][q->inv[1] + sp->cost[1]][q->inv[2] + sp->cost[2]][q->inv[3] + sp->cost[3]][0] = new->sum;
                    a = 0;
                    while (a < 29 - multiple && q->path[a] != 0) {
                        new->path[a] = q->path[a];
                        a++;
                    }
                    save = a;
                    while (a < save + multiple)
                        new->path[a++] = sp->id;
                    for (int i = 0; i < 20; i++)
                        new->spell[i] = q->spell[i];
                    new->spell[sp->sid] = false;
                    new->path[a] = 0;

                } else
                    break;
            }
            //same with reload
            if (!q->spell[sp->sid] && launchable(sp, q, 1)
            && visited[q->inv[0] + sp->cost[0]][q->inv[1] + sp->cost[1]][q->inv[2] + sp->cost[2]][q->inv[3] + sp->cost[3]][0] > q->sum
            && diff_state(q, 1, visited[q->inv[0] + sp->cost[0]][q->inv[1] + sp->cost[1]][q->inv[2] + sp->cost[2]][q->inv[3] + sp->cost[3]], 1, first_spell))
            {
                new = graveyard(NULL);
                new->next = NULL;
                if (last_q != NULL)
                    last_q->next = new;
                else
                    first_q = new;
                last_q = new;

                a = 0;
                while (a < 29 && q->path[a] != 0) {
                    new->path[a] = q->path[a];
                    a++;
                }
                new->path[a] = -1;
                new->path[a + 1] = 0;

                new->inv[0] = q->inv[0];
                new->inv[1] = q->inv[1];
                new->inv[2] = q->inv[2];
                new->inv[3] = q->inv[3];
                new->sum = q->sum + 1;
                for (int i = 0; i < 20; i++)
                    new->spell[i] = true;
            }
            sp = sp->next;
        }
        graveyard(q);
    }
}

// print int to stdout
void put_nbr(int n)
{
    if (n >= 0)
        n *= -1;
    else
        putchar('-');
    if (n < -9)
        put_nbr(-(n / 10));
    putchar(-(n % 10) + '0');
}

// returns true if _inv >= _cost
bool got_cost(int cost[4], char inv[4]) {
    for (int i = 0; i < 4; i++) {
        if (cost[i] + inv[i] < 0)
            return false;
    }
    return true;
}

// Add new spell to spell list
spell_t *add_spell(spell_t *sp, int cost[4], int id, bool _repeatable, bool _usable) {
    if (sp == NULL) {
        sp = malloc(sizeof(spell_t));
        sp->sid = 0;
        sp->prev = NULL;
        sp->next = NULL;
    } else {
        sp->next = malloc(sizeof(spell_t));
        sp->next->prev = sp;
        sp = sp->next;
    }
    sp->cost[0] = cost[0];
    sp->cost[1] = cost[1];
    sp->cost[2] = cost[2];
    sp->cost[3] = cost[3];
    sp->id = id;
    if (sp->prev)
        sp->sid = sp->prev->sid + 1;
    sp->repeatable = _repeatable;
    sp->usable = _usable;
    sp->next = NULL;
    return sp;
}

// Add new learnable spell to learnable list
learnable_t *add_learnable(int cost[4], int id, learnable_t *ls, int taxe, bool _repeatable, int index) {
    if (ls == NULL) {
        ls = malloc(sizeof(learnable_t));
        ls->prev = NULL;
        ls->next = NULL;
    } else {
        ls->next = malloc(sizeof(learnable_t));
        ls->next->prev = ls;
        ls = ls->next;
    }
    ls->cost[0] = cost[0];
    ls->cost[1] = cost[1];
    ls->cost[2] = cost[2];
    ls->cost[3] = cost[3];
    ls->id = id;
    ls->index = index;
    ls->repeatable = _repeatable;
    ls->taxe = taxe;
    ls->next = NULL;
    return ls;
}

// print cast to stdout
void cast(int id, spell_t *sp, int nb) {
    write(1, "CAST ", 5);
    put_nbr(id);
    write(1, " ", 1);
    put_nbr(nb);
    write(1, "\n", 1);
    sp->usable = false;
}

// print rest to stdout, reload spells
void rest(spell_t *first_sp) {
    while (first_sp != NULL) {
        first_sp->usable = true;
        first_sp = first_sp->next;
    }
    write(1, "REST \n", 6);
}

void spelling(pot_t *potion, spell_t *first_sp, char inv[4]) {
    int i = 0;
    while (potion->path[++i] == potion->path[0])
        ;
    if (potion->path[0] > 0) {
        while (first_sp->id != potion->path[0])
            first_sp = first_sp->next;
        cast(potion->path[0], first_sp, i);
        return;  
    } else if (potion->path[0] == -1) {
        rest(first_sp);
    } else {
        write(1, "BREW ", 5);
        put_nbr(potion->id);
        write(1, "\n", 1);
        brewed += 1;
        objective = -1;
    }
}

void reset(pot_t *pots, spell_t *spells, int id) {
    while (pots != NULL) {
        pots->path[0] = 0;
        pots->got_path = false;
        pots = pots->next;
    }
    while (spells->id != id)
        spells = spells->next;
    if (spells->prev)
        spells->prev->next = spells->next;
    if (spells->next)
        spells->next->prev = spells->prev;
    free(spells);
}

void add_spell_end(learnable_t *ls, spell_t *spells) {
    while (spells->next)
        spells = spells->next;
    add_spell(spells, ls->cost, ls->id, ls->repeatable, 1);
}

void set_new_spells(learnable_t *ls, spell_t *spells, pot_t *pots, char *my_inv) {
    int sum;
    float sum_value;
    pot_t *save_pots;
    int best_sum = 1000;
    char inv[4];
    int inv_size = 0;

    for (int i = 0; i < 4; i++) {
        inv[i] = my_inv[i];
        inv_size += inv[i];
    }
    while (ls != NULL) {
        sum = 0;
        sum_value = 0;
        if (inv[0] >= ls->index){
            add_spell_end(ls, spells);
            if (inv_size + ls->taxe - ls->index > 10)
                inv[0] += 10 - inv_size;
            else
                inv[0] -= ls->index - ls->taxe;
            bfs(inv, spells, 5, pots);
            save_pots = pots;
            while (save_pots) {
                sum += save_pots->value * save_pots->price;
                sum_value += save_pots->value;
                save_pots = save_pots->next;
            }
            ls->sum = sum;
            ls->sum_value = sum_value;
            fprintf(stderr, "sum value = %d %f\n", sum, sum_value);
            inv[0] += ls->index - ls->taxe;
            reset(pots, spells, ls->id);
        }
        ls = ls->next;
    }
}

int buy_spell(learnable_t *ls, int spells_learned, char inv[4], int actual_sum) {
    learnable_t *save;
    float worst_sum_value = -1;
    float best_sum_value = 1000;
    int best_sum = 1000;
    int worst_sum = -1;
    int best_id;
    int best_cost;
    int best_id2;
    int best_cost2;
    int sum = 100;

    if (spells_learned > 9)
        return -1;

    save = ls;
    while (save != NULL) {
        if (save->sum_value > worst_sum_value)
            worst_sum_value = save->sum_value;
        if (save->sum > worst_sum)
            worst_sum = save->sum;
        if (save->sum < best_sum && save->index <= inv[0]) {
            best_sum = save->sum;
            best_id2 = save->id;
            best_cost2 = save->index;
        }
        if (save->sum_value < best_sum_value && save->index <= inv[0]) {
            best_sum_value = save->sum_value;
            best_id = save->id;
            best_cost = save->index;
        }
        save = save->next;
    }
    //fprintf(stderr, "%f\n", 10 * worst_sum_value / best_sum_value);
    //fprintf(stderr, "%d %d\n", best_sum, worst_sum);
    fprintf(stderr, "actual = %d, best = %d\n", actual_sum, best_sum);
    if ((actual_sum - best_sum) > (spells_learned > 4 ? 4 : 1))
        return best_id2;
    return -1;
}

int main()
{
    //sum all pot operations
    int actual_sum;
    //minimum enemy time to finish
    int min_en_time;
    //spell to buy
    int buy_id;
    clock_t timer;
    // the number of spells and recipes in play
    int action_count;

    // the unique ID of this spell or recipe
    int action_id;
    // in the first league: BREW; later: CAST, OPPONENT_CAST, LEARN, BREW
    char action_type[21];
    // tier-i ingredient change
    int delta[4];

    // the price in rupees if this is a potion
    int price;
    // in the first two leagues: always 0; later: the index in the tome if this is a tome spell, equal to the read-ahead tax; For brews, this is the value of the current urgency bonus
    int tome_index;
    // in the first two leagues: always 0; later: the amount of taxed tier-0 ingredients you gain from learning this spell; For brews, this is how many times you can still gain an urgency bonus
    int tax_count;
    // in the first league: always 0; later: 1 if this is a castable player spell
    int _castable;
    // for the first two leagues: always 0; later: 1 if this is a repeatable player spell
    int _repeatable;

    // tier-0 ingredients in inventory
    char inv[4];
    // amount of rupees
    int score;

    // enemy tier-0 ingredients in inventory
    char en_inv[4];
    // enemy amount of rupees
    int en_score;
    int en_save_score = 0;
    int brew_time = 0;

    pot_t *save;
    learnable_t *lsave;
    spell_t *ssave;

    //list of pots
    pot_t *it;
    pot_t *it_cpy;
    //list of spells
    spell_t *sp = NULL;
    spell_t *en_sp = NULL;
    learnable_t *ls = NULL;
    //learned a spell
    bool learned = true;

    int turn = 0;
    int spells_learned = 0;
    int av;
    int avnext;
    int avnextnext;

    float best_time;
    float best_price;
    float best_value;
    int best_id;
    bool objective_sold;

    // game loop
    while (1) {
        it = malloc(sizeof(pot_t));
        it->next = NULL;
        it->prev = NULL;
        it->price = -1;

        it_cpy = malloc(sizeof(pot_t));
        it_cpy->next = NULL;
        it_cpy->prev = NULL;
        it_cpy->price = -1;


        scanf("%d", &action_count);

        for (int i = 0; i < action_count; i++) {
            scanf("%d%s%d%d%d%d%d%d%d%d%d", &action_id, action_type, delta, delta+1, delta+2, delta+3, &price, &tome_index, &tax_count, &_castable, &_repeatable);
            //fprintf(stderr, "%d %s %d %d %d %d %d %d %d %d %d\n", action_id, action_type, delta[0], delta[1], delta[2], delta[3], price, tome_index, tax_count, _castable, _repeatable);
            
            if (learned && strcmp(action_type, "CAST") == 0) {
                sp = add_spell(sp, delta, action_id, (bool)_repeatable, (bool)_castable);
            }
            else if (!strcmp(action_type, "LEARN")) {
                ls = add_learnable(delta, action_id, ls, tax_count, _repeatable, tome_index);
            }
            else if (!strcmp(action_type, "OPPONENT_CAST")) {
                en_sp = add_spell(en_sp, delta, action_id, (bool)_repeatable, (bool)_castable);
            }
            else if (!strcmp(action_type, "BREW")) {
                if (it->price != -1){
                    it->next = malloc(sizeof(pot_t));
                    it->next->prev = it;
                    it = it->next;
                }
                it->cost[0] = delta[0];
                it->cost[1] = delta[1];
                it->cost[2] = delta[2];
                it->cost[3] = delta[3];
                it->price = price;
                it->id = action_id;
                it->got_path = false;
                it->next = NULL;

                if (it_cpy->price != -1){
                    it_cpy->next = malloc(sizeof(pot_t));
                    it_cpy->next->prev = it_cpy;
                    it_cpy = it_cpy->next;
                }
                it_cpy->cost[0] = delta[0];
                it_cpy->cost[1] = delta[1];
                it_cpy->cost[2] = delta[2];
                it_cpy->cost[3] = delta[3];
                it_cpy->price = price;
                it_cpy->id = action_id;
                it_cpy->got_path = false;
                it_cpy->next = NULL;
            }
        }

        scanf("%d%d%d%d%d", inv, inv + 1, inv + 2, inv + 3, &score);
        //fprintf(stderr, "%d%d%d%d%d\n", inv, inv + 1, inv + 2, inv + 3, score);
        scanf("%d%d%d%d%d", en_inv, en_inv + 1, en_inv + 2, en_inv + 3, &en_score);
        //fprintf(stderr, "%d%d%d%d%d\n", en_inv_0, en_inv_1, en_inv_2, en_inv_3, en_score);

        timer = clock();

        if (en_save_score < en_score){
            en_save_score = en_score;
            brew_time += 1;
        }

        learned = false;

        //Every list on first
        while (it->prev != NULL)
            it = it->prev;
        while (it_cpy->prev != NULL)
            it_cpy = it_cpy->prev;

        while (sp->prev != NULL)
            sp = sp->prev;
        while (en_sp->prev != NULL)
            en_sp = en_sp->prev;

        while (ls->prev != NULL)
            ls = ls->prev;
        ///////////////////////

        //set spells
        fprintf(stderr, "---Set spells---\n");
        set_new_spells(ls, sp, it, inv);
        /*fprintf(stderr, "Spell %i is %i and %f\n", ls->id, ls->sum, ls->sum_value); 
        while (ls->next) {
            ls = ls->next;
            fprintf(stderr, "Spell %i is %i and %f\n", ls->id, ls->sum, ls->sum_value);
        }
        while (ls->prev != NULL)
            ls = ls->prev;*/

        //set values of potions
        fprintf(stderr, "---me---\n");
        bfs(inv, sp, 5, it);

        //sum of all operations
        actual_sum = 0;
        while (it->next != NULL) {
            actual_sum += it->value * it->price;
            it = it->next;
        }
        actual_sum += it->value * it->price;
        while (it->prev != NULL)
            it = it->prev;

        //Find best value ...
        save = it;
        //... if we are ending
        if (score - en_score > 20)
        {
            best_time = 1000;
            save = it;
            while (save != NULL) {
                if (save->price * save->value <= best_time && save->price > best_price) {
                    best_time = save->price * save->value;
                    best_price = save->price;
                    best_id = save->id;
                }
                save = save->next;
            }

        }
        else if (brewed == 5 || brew_time == 5)
        {
            best_time = 1000;
            best_price = 0;
            fprintf(stderr, "---him---\n");
            bfs(en_inv, en_sp, 5, it_cpy);
            min_en_time = 1000;
            save = it_cpy;
            while (save != NULL) {
                if (min_en_time > save->value * save->price)
                    min_en_time = save->value * save->price;
                save = save->next;
            }
            if (brewed == 5)
                min_en_time--;
            //strat end faster and pricer
            save = it;
            while (save != NULL) {
                if (save->price > best_price && save->value * save->price <= min_en_time) {
                    best_price = save->price;
                    best_id = save->id;
                }
                save = save->next;
            }
            //if I'm ending, end pricer
            if (best_price == 0 && brewed == 5 && brew_time != 5) {
                save = it;
                while (save != NULL) {
                    if (save->price > best_price) {
                        best_price = save->price;
                        best_id = save->id;
                    }
                    save = save->next;
                }
            }
            //if I'm ending and he's ending, end faster
            else if (best_price == 0 && brewed == 5 && brew_time == 5) {
                save = it;
                while (save != NULL) {
                    if (save->price * save->value <= best_time && save->price > best_price) {
                        best_time = save->price * save->value;
                        best_price = save->price;
                        best_id = save->id;
                    }
                    save = save->next;
                }
            }
            //if he's ending, strat ???
            else if (best_price == 0) {
                save = it;
                while (save != NULL) {
                    if (save->price > best_price) {
                        best_price = save->price;
                        best_id = save->id;
                    }
                    save = save->next;
                }
            }
            while (it->id != best_id)
                it = it->next;
            objective = it->id;
        } 
        //... if not ending
        else
        {
            objective_sold = true;
            best_value = 1000;
            while (save != NULL) {
                if (save->value < best_value && save->value * save->price < 11)
                    best_value = save->value;
                if (objective == save->id)
                    objective_sold = false;
                save = save->next;
            }
            if (objective == -1 || objective_sold){
                while (it->next && it->value != best_value)
                    it = it->next;
                objective = it->id;
            }
            else {
                while (it->id != objective)
                    it = it->next;
            }
        }

        fprintf(stderr, "objective : %d\n", objective);


        //Take action !

        // Check if we buy a spell
        if ((buy_id = buy_spell(ls, spells_learned, inv, actual_sum)) != -1)
        {
            write(1, "LEARN ", 6);
            put_nbr(buy_id);
            write(1, "\n", 1);
            learned = true;
            spells_learned += 1;
            objective = -1;
        }
        else
            spelling(it, sp, inv);

        //free memory for potions
        while (it->next != NULL)
            it = it->next;
        while (it != NULL) {
            save = it;
            it = it->prev;
            free(save);
        }
        while (it_cpy->next != NULL)
            it_cpy = it_cpy->next;
        while (it_cpy != NULL) {
            save = it_cpy;
            it_cpy = it_cpy->prev;
            free(save);
        }
        //free memory for learnable spells
        while (ls->next != NULL)
            ls = ls->next;
        while (ls != NULL) {
            lsave = ls;
            ls = ls->prev;
            free(lsave);
        }
        //free memory for spells
        if (learned) {
            while (sp != NULL) {
                ssave = sp;
                sp = sp->next;
                free(ssave);
            }
        }
        while (en_sp != NULL) {
            ssave = en_sp;
            en_sp = en_sp->next;
            free(ssave);
        }

        fprintf(stderr, "Found solution in %ld ns\n", clock() - timer); 

        turn++;
    }

    return 0;
}