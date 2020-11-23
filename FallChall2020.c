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

typedef struct pot {
    int cost[4];
    int id;
    int price;
    float value;
    int path[30];
    bool got_path;
    struct pot *next;
    struct pot *prev;
} pot;

typedef struct spell {
    int cost[4];
    int id;
    char sid;
    bool usable;
    bool repeatable;
    struct spell *next;
    struct spell *prev;
} spell;

typedef struct learnable {
    int cost[4];
    int id;
    int taxe;
    int sum;
    float sum_value;
    bool repeatable;
    int index;
    struct learnable *next;
    struct learnable *prev;
} learnable;

typedef struct queue {
    char inv[4];
    char path[30];
    bool spell[20];
    char sum;
    struct queue *next;
} queue;

char can_do_pot(queue *q, pot *pot) {
    bool can_do_pot;
    char a = 0;
    char i;
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
            pot->got_path = true;
            a += 1;
            i = 0;
            while (q->path[i]) {
                fprintf(stderr,"%d ", q->path[i]);
                pot->path[i] = q->path[i];
                i++;
            }
            pot->path[i] = q->path[i];
            pot->value = (float)q->sum / pot->price;
            fprintf(stderr,"   Val %.4f Price %d Id %d Op %.0f\n", pot->value, pot->price, pot->id, (float)q->sum);

        }
        pot = pot->next;
    }
    return a;
}

bool launchable(spell *sp, queue *q, char multiple) {
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

bool diff_path(queue *q, int sum, int roads[40], char multiple, spell *spells) {
    bool b;
    int i;
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

    i = 1;
    while (roads[i]) {
        if (sum == roads[i])
            return false;
        i++;
    }

    while(roads[i] && i < 38)
        i++;
    roads[i] = sum;
    roads[i + 1] = 0;
    return true;
}

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

void bfs(char *inv, spell *first_spell, char nb_pots, pot *first_pot) {
    int cmp_pots = 0;
    char multiple;
    queue *q;
    queue *new;
    int a;
    int save;
    spell *sp;

    clock_t timer = clock(); 

    //set visited
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
    visited[inv[0]][inv[1]][inv[2]][inv[3]][0] = 0;

    queue *lastQ;
    queue *firstQ;
    firstQ = graveyard(NULL);
    firstQ->inv[0] = inv[0];
    firstQ->inv[1] = inv[1];
    firstQ->inv[2] = inv[2];
    firstQ->inv[3] = inv[3];
    firstQ->path[0] = 0;
    firstQ->sum = 0;
    firstQ->next = NULL;
    sp = first_spell;
    while (sp != NULL) {
        firstQ->spell[sp->sid] = sp->usable;
        sp = sp->next;
    }
    lastQ = firstQ;

    int cmp = 0;
    while (firstQ != NULL){
        cmp++;
        q = firstQ;
        firstQ = firstQ->next;
        if (q == lastQ)
            lastQ = NULL;

        cmp_pots += can_do_pot(q, first_pot);
        if (cmp_pots == nb_pots) {
            fprintf(stderr, "cmp = %d\n", cmp);
            clock_t time_spent = clock() - timer;
            fprintf(stderr, "time spent %ldns average %lfns\n", time_spent, (double)time_spent / cmp); 
            //free all list
            graveyard(q);
            while(firstQ != NULL) {
                q = firstQ;
                firstQ = firstQ->next;
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
                && diff_path(q, -sp->id, visited[q->inv[0] + multiple * sp->cost[0]][q->inv[1] + multiple * sp->cost[1]][q->inv[2] + multiple * sp->cost[2]][q->inv[3] + multiple * sp->cost[3]], multiple, first_spell))
                {
                    new = graveyard(NULL);
                    new->next = NULL;
                    if (lastQ != NULL)
                        lastQ->next = new;
                    else
                        firstQ = new;
                    lastQ = new;

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
            && diff_path(q, 1, visited[q->inv[0] + sp->cost[0]][q->inv[1] + sp->cost[1]][q->inv[2] + sp->cost[2]][q->inv[3] + sp->cost[3]], 1, first_spell))
            {
                new = graveyard(NULL);
                new->next = NULL;
                if (lastQ != NULL)
                    lastQ->next = new;
                else
                    firstQ = new;
                lastQ = new;

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

void     put_nbr(int n)
{
    if (n < 0) {
        putchar('-');
        n = -n;
    }
    if (n >= 10) {
        put_nbr(n / 10);
        putchar(n % 10 + '0');
    }
    if (n < 10)
        putchar(n % 10 + '0');
}

bool got_cost(int cost[4], char inv[4]) {
    for (int i = 0; i < 4; i++) {
        if (cost[i] + inv[i] < 0)
            return false;
    }
    return true;
}

spell *add_spell(int cost[4], int id, spell *sp, bool _repeatable, bool _usable) {
    if (sp == NULL) {
        sp = malloc(sizeof(spell));
        sp->sid = 0;
        sp->prev = NULL;
        sp->next = NULL;
    } else {
        sp->next = malloc(sizeof(spell));
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

learnable *add_learnable(int cost[4], int id, learnable *ls, int taxe, bool _repeatable, int index) {
    if (ls == NULL) {
        ls = malloc(sizeof(learnable));
        ls->prev = NULL;
        ls->next = NULL;
    } else {
        ls->next = malloc(sizeof(learnable));
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

void cast(int id, spell *sp, int nb) {
    write(1, "CAST ", 5);
    put_nbr(id);
    write(1, " ", 1);
    put_nbr(nb);
    write(1, "\n", 1);
    sp->usable = false;
}

void rest(spell *first_sp) {
    while (first_sp != NULL) {
        first_sp->usable = true;
        first_sp = first_sp->next;
    }
    write(1, "REST \n", 6);
}

void spelling(pot *potion, spell *first_sp, char inv[4]) {
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

void reset(pot *pots, spell *spells, int id) {
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

void add_spell_end(learnable *ls, spell *spells) {
    while (spells->next)
        spells = spells->next;
    add_spell(ls->cost, ls->id, spells, ls->repeatable, 1);
}

void set_new_spells(learnable *ls, spell *spells, pot *pots, char *my_inv) {
    int sum;
    float sum_value;
    pot *save_pots;
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

int buy_spell(learnable *ls, int spells_learned, char inv[4], int actual_sum) {
    learnable *save;
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

    pot *save;
    learnable *lsave;
    spell *ssave;

    //list of pots
    pot *it;
    pot *it_cpy;
    //list of spells
    spell *sp = NULL;
    spell *en_sp = NULL;
    learnable *ls = NULL;
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
        it = malloc(sizeof(pot));
        it->next = NULL;
        it->prev = NULL;
        it->price = -1;

        it_cpy = malloc(sizeof(pot));
        it_cpy->next = NULL;
        it_cpy->prev = NULL;
        it_cpy->price = -1;


        scanf("%d", &action_count);

        for (int i = 0; i < action_count; i++) {
            scanf("%d%s%d%d%d%d%d%d%d%d%d", &action_id, action_type, delta, delta+1, delta+2, delta+3, &price, &tome_index, &tax_count, &_castable, &_repeatable);
            //fprintf(stderr, "%d %s %d %d %d %d %d %d %d %d %d\n", action_id, action_type, delta[0], delta[1], delta[2], delta[3], price, tome_index, tax_count, _castable, _repeatable);
            
            if (learned && strcmp(action_type, "CAST") == 0) {
                sp = add_spell(delta, action_id, sp, (bool)_repeatable, (bool)_castable);
            }
            else if (!strcmp(action_type, "LEARN")) {
                ls = add_learnable(delta, action_id, ls, tax_count, _repeatable, tome_index);
            }
            else if (!strcmp(action_type, "OPPONENT_CAST")) {
                en_sp = add_spell(delta, action_id, en_sp, (bool)_repeatable, (bool)_castable);
            }
            else if (!strcmp(action_type, "BREW")) {
                if (it->price != -1){
                    it->next = malloc(sizeof(pot));
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
                    it_cpy->next = malloc(sizeof(pot));
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