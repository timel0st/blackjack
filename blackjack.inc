#include <YSI_Coding\y_hooks>

#if !defined MAX_CARD_SPRITE_LEN
    #define MAX_CARD_SPRITE_LEN (16)
#endif
#if !defined MAX_CARD_NAME_LEN
    #define MAX_CARD_NAME_LEN (18)
#endif

#define INVALID_PLAYER_TEXT_ID PlayerText:-1
#define CARD_BACK_SPRITE_NAME "LD_CARD:cdback"
#define CARD_STACK_SIZE 52 // Размер колоды
#define CARD_SET_SIZE 13 // Кол-во карт в пределах одной масти
#define BJ_MAX_HAND_SIZE 21 // Макс число карт в руке (худший сценарий)
#define BJ_DEALER_STAYS_ON 17 // На каком количестве очков останавливается дилер
#define BJ_MIN_STAKE 100 // Минимальная ставка
#define BJ_MAX_STAKE 100000 // Максимальная ставка
#define BJ_DRAW_DELAY_MS 1000 // Задержка между вытягиваниями карт, в мс

enum e_CARD_SETS { 
    CARD_SET_SPADES, 
    CARD_SET_HEARTS, 
    CARD_SET_DIAMONDS, 
    CARD_SET_CLOVERS,
    CARD_SET_BACK
}; 

enum e_CARD_VALUES {
    CARD_VALUE_ACE,
    CARD_VALUE_TWO,
    CARD_VALUE_THREE,
    CARD_VALUE_FOUR,
    CARD_VALUE_FIVE,
    CARD_VALUE_SIX,
    CARD_VALUE_SEVEN,
    CARD_VALUE_EIGHT,
    CARD_VALUE_NINE,
    CARD_VALUE_TEN,
    CARD_VALUE_JACK,
    CARD_VALUE_QUEEN,
    CARD_VALUE_KING
};

enum E_CARD_SPRITES {
    E_CARD_SPRITE[MAX_CARD_SPRITE_LEN],
    CardValues: E_CARD_VALUE,
    CardSets: E_CARD_SET
}

new const Float:BJ_BLACKJACK_MULTIPLIER = 1.5; // Множитель выигрыша при блекджеке

// Возвращает название для заданной карты
stock GetPlayingCardSpriteName(name[], value, set) {
    new setLetter;

    switch (set) {
        case CARD_SET_SPADES:
            setLetter = 's';
        case CARD_SET_CLOVERS:
            setLetter = 'c';
        case CARD_SET_DIAMONDS:
            setLetter = 'd';
        case CARD_SET_HEARTS:
            setLetter = 'h';
        default:
            setLetter = 'b';
    }

    if (setLetter == 'b') {
        format(name, MAX_CARD_SPRITE_LEN, CARD_BACK_SPRITE_NAME);
        return 0;
    }
    format(name, MAX_CARD_SPRITE_LEN, "LD_CARD:cd%d%c", value + 1, setLetter);
    // print(name);
    return 0;
}

// Blackjack player types
enum e_BJ_PLAYERS {
    BJ_DEALER,
    BJ_PLAYER,
    BJ_PLAYER_SPLIT_FIRST,
    BJ_PLAYER_SPLIT_SECOND,
    BJ_PLAYERS_AMOUNT
};

// Changable textdraws
enum e_BJ_TEXTDRAWS {
    PlayerText:BJ_DEALER_SCORE_TD,
    PlayerText:BJ_PLAYER_SCORE_TD,
    PlayerText:BJ_PLAYER_SPLIT_FIRST_SCORE_TD,
    PlayerText:BJ_PLAYER_SPLIT_SECOND_SCORE_TD,
    PlayerText:BJ_TIMER_TD,
    PlayerText:BJ_HIT_TEXTDRAW,
    PlayerText:BJ_DOUBLE_TEXTDRAW,
    PlayerText:BJ_STAY_TEXTDRAW,
    PlayerText:BJ_SPLIT_TEXTDRAW,
    BJ_PLAYER_TEXTDRAWS_AMOUNT
};

enum E_BJ_GAME {
    BJ_DealerHand[BJ_MAX_HAND_SIZE], // Карты дилера
    BJ_PlayerHand[BJ_MAX_HAND_SIZE], // Карты игрока
    BJ_PlayerFirstHand[BJ_MAX_HAND_SIZE], // Карты игрока (после сплита)
    BJ_PlayerSecondHand[BJ_MAX_HAND_SIZE], // Карты игрока (после сплита)
    BJ_CardsAmount[BJ_PLAYERS_AMOUNT], // Кол-во карт у игроков
    BJ_Scores[BJ_PLAYERS_AMOUNT], // Очки игроков
    bool:BJ_IsActive, // Статус игры
    bool:BJ_IsControlsActive, // Статус курсора у игрока
    PlayerText:BJ_PlayerTextdraws[BJ_PLAYER_TEXTDRAWS_AMOUNT], // Текстдравы (кроме карт)
    PlayerText:BJ_DealerCardsTDs[BJ_MAX_HAND_SIZE], // Текстдравы карт дилера
    PlayerText:BJ_PlayerCardsTDs[BJ_MAX_HAND_SIZE], // Текстдравы карт игрока
    PlayerText:BJ_PlayerFirstCardsTDs[BJ_MAX_HAND_SIZE], // Текстдравы карт игрока (после сплита)
    PlayerText:BJ_PlayerSecondCardsTDs[BJ_MAX_HAND_SIZE], // Текстдравы карт игрока (после сплита)
    BJ_Stakes[3], // Ставки 
    e_BJ_PLAYERS:BJ_Turn // Чей ход?
};

new
    BJ_Games[MAX_PLAYERS][E_BJ_GAME];

// Начало игры, инициализация
stock BJ_StartGame(playerid, stake) {
    BJ_Games[playerid][BJ_Stakes] = {0, 0, 0};
    BJ_Games[playerid][BJ_Stakes][0] = stake;
    BJ_Games[playerid][BJ_Turn] = BJ_PLAYER;
    for (new j = 0; j < 2; j++) {
        BJ_Games[playerid][BJ_DealerHand][j] = CARD_STACK_SIZE;
        BJ_Games[playerid][BJ_PlayerHand][j] = CARD_STACK_SIZE;
    }
    for (new i = 0; i < BJ_MAX_HAND_SIZE; i++) {
        BJ_Games[playerid][BJ_DealerCardsTDs][i] = INVALID_PLAYER_TEXT_ID;
        BJ_Games[playerid][BJ_PlayerCardsTDs][i] = INVALID_PLAYER_TEXT_ID;
        BJ_Games[playerid][BJ_PlayerFirstCardsTDs][i] = INVALID_PLAYER_TEXT_ID;
        BJ_Games[playerid][BJ_PlayerSecondCardsTDs][i] = INVALID_PLAYER_TEXT_ID;
    }
    for (new e_BJ_TEXTDRAWS:i = BJ_DEALER_SCORE_TD; i < BJ_PLAYER_TEXTDRAWS_AMOUNT; i++) {
        BJ_Games[playerid][BJ_PlayerTextdraws][i] = INVALID_PLAYER_TEXT_ID;
    }
    BJ_DrawTable(playerid);
    BJ_Games[playerid][BJ_CardsAmount] = {2, 2, 0, 0};
    BJ_Games[playerid][BJ_Scores] = {0, 0, 0, 0};
    BJ_Games[playerid][BJ_IsActive] = true;
    BJ_Games[playerid][BJ_IsControlsActive] = false;
    BJ_Render_Cards(playerid, BJ_DEALER);
    BJ_Render_Cards(playerid, BJ_PLAYER);
    SetTimerEx("BJ_FlipCard", BJ_DRAW_DELAY_MS, false, "iii", playerid, BJ_DEALER, 0);
    SetTimerEx("BJ_FlipCard", 2*BJ_DRAW_DELAY_MS, false, "iii", playerid, BJ_PLAYER, 0);
    SetTimerEx("BJ_FlipCard", 3*BJ_DRAW_DELAY_MS, false, "iii", playerid, BJ_PLAYER, 1);
    //SetTimerEx("BJ_SetControlsActive", 3000, false, "ii", playerid, true);
}

// Отрисовка текстдравов для игры
stock BJ_DrawTable(playerid) {
    BJ_DrawPlayerButtons(playerid);
}

// Скрытие текстдравов, перевод в неактивное состояние
forward BJ_HideTable(playerid);
public BJ_HideTable(playerid) {
    for (new e_BJ_TEXTDRAWS:i = BJ_DEALER_SCORE_TD; i < BJ_PLAYER_TEXTDRAWS_AMOUNT; i++) {
        PlayerTextDrawHide(playerid, BJ_Games[playerid][BJ_PlayerTextdraws][i]);
        PlayerTextDrawDestroy(playerid, BJ_Games[playerid][BJ_PlayerTextdraws][i]);
    }
    for (new e_BJ_PLAYERS:i = BJ_DEALER; i < BJ_PLAYERS_AMOUNT; i++) {
        BJ_DeleteCards(playerid, i);
    }
    BJ_Games[playerid][BJ_IsActive] = false;
}


stock BJ_CountHand(playerid, e_BJ_PLAYERS:hand) {
    new player_score = BJ_Games[playerid][BJ_Scores][hand];
    new stake = BJ_Games[playerid][BJ_Stakes][hand - BJ_PLAYER];
    new dealer_score = BJ_Games[playerid][BJ_Scores][BJ_DEALER];
    new isPlayerBJ = (player_score == 21 && BJ_Games[playerid][BJ_CardsAmount][hand] == 2);
    new isDealerBJ = (dealer_score == 21 && BJ_Games[playerid][BJ_CardsAmount][BJ_DEALER] == 2);
    if (player_score > 21 || (player_score < dealer_score && dealer_score <= 21) || (isDealerBJ && !isPlayerBJ)) {
        SendClientMessage(playerid, 0xFFFFFFFF, "Вы проиграли!");
    } else if (player_score == dealer_score && isPlayerBJ == isDealerBJ) {
        SendClientMessage(playerid, 0xFFFFFFFF, "Ничья! (+%d$)", stake);
        AddPlayerMoney(playerid, stake);
    } else if (isPlayerBJ) {
        new Float:win = float(stake)*BJ_BLACKJACK_MULTIPLIER;
        stake = stake + floatround(win, floatround_floor);
        SendClientMessage(playerid, 0xFFFFFFFF, "BLACKJACK! (+%d$)", stake);
        AddPlayerMoney(playerid, stake);
    } else {
        stake *= 2;
        SendClientMessage(playerid, 0xFFFFFFFF, "Вы победили! (+%d$)", stake);
        AddPlayerMoney(playerid, stake);
    }
}

// Завершение игры, подсчет результата
stock BJ_EndGame(playerid) {
    if (BJ_Games[playerid][BJ_Scores][BJ_PLAYER_SPLIT_FIRST] > 0) {
        BJ_CountHand(playerid, BJ_PLAYER_SPLIT_FIRST);
        BJ_CountHand(playerid, BJ_PLAYER_SPLIT_SECOND);
    } else {
        BJ_CountHand(playerid, BJ_PLAYER);
    }
    SetTimerEx("BJ_HideTable", 2000, false, "i", playerid);
}

// Обновление текстдрава с очками
stock BJ_UpdateScoreTD(playerid, e_BJ_PLAYERS:playertype, score) {
    PlayerTextDrawSetString(
        playerid,
        BJ_Games[playerid][BJ_PlayerTextdraws][e_BJ_TEXTDRAWS:playertype],
        "%s: %d",
        playertype == BJ_DEALER ? "Dealer" : "You",
        score
    );
}

// Получение достоинства карты
stock BJ_GetCardValue(cardidx) {
    return min((cardidx % 13) + 1, 10);
}

stock BJ_ProcessDraw(playerid, e_BJ_PLAYERS:playertype) {
    new sum = BJ_Games[playerid][BJ_Scores][playertype];
    if (playertype == BJ_DEALER) {
        if (sum >= 21) {
            BJ_EndGame(playerid);
        } else {
            BJ_DealerTurn(playerid);
        }
    } else {
        if (sum >= 21) {
            BJ_EndPlayerTurn(playerid, playertype);
        }
    }
    return sum;
}

// Пересчет суммы очков 
stock BJ_UpdateSum(playerid, e_BJ_PLAYERS:playertype) {
    new sum = 0;
    new bool:isAce = false;
    for (new i = 0; i < BJ_Games[playerid][BJ_CardsAmount][playertype]; i++) {
        new cardidx;
        switch (playertype) {
            case BJ_DEALER:
                cardidx = BJ_Games[playerid][BJ_DealerHand][i];
            case BJ_PLAYER:
                cardidx = BJ_Games[playerid][BJ_PlayerHand][i];
            case BJ_PLAYER_SPLIT_FIRST:
                cardidx = BJ_Games[playerid][BJ_PlayerFirstHand][i];
            case BJ_PLAYER_SPLIT_SECOND:
                cardidx = BJ_Games[playerid][BJ_PlayerSecondHand][i];
        }
        if (cardidx / 13 > 3) {
            continue;
        }
        new val = min((cardidx % 13) + 1, 10);
        if (val == 1) {
            isAce = true;
        }
        sum += val;
    }
    if (isAce && sum <= 11) {
        sum += 10;
    }
    BJ_Games[playerid][BJ_Scores][playertype] = sum;
    // Update tds?
    BJ_UpdateScoreTD(playerid, playertype, sum);
}

// Ход дилера
stock BJ_DealerTurn(playerid) {
    new dealerScore = BJ_Games[playerid][BJ_Scores][BJ_DEALER];
    if (dealerScore < BJ_DEALER_STAYS_ON) {
        SetTimerEx("BJ_Hit", BJ_DRAW_DELAY_MS, false, "ii", playerid, BJ_DEALER);
    } else {
        SetTimerEx("BJ_Stay", BJ_DRAW_DELAY_MS, false, "ii", playerid, BJ_DEALER);
    }
}

// "Переворот" карты
forward BJ_FlipCard(playerid, e_BJ_PLAYERS:playertype, cardidx);
public BJ_FlipCard(playerid, e_BJ_PLAYERS:playertype, cardidx) {
    switch (playertype) {
        case BJ_DEALER:
            BJ_Games[playerid][BJ_DealerHand][cardidx] = random(CARD_STACK_SIZE);
        case BJ_PLAYER:
            BJ_Games[playerid][BJ_PlayerHand][cardidx] = random(CARD_STACK_SIZE);
        default:
            return 1;
    }
    BJ_Render_Cards(playerid, playertype);
    BJ_UpdateSum(playerid, playertype);
    if (!(playertype == BJ_DEALER && cardidx == 0))
        BJ_ProcessDraw(playerid, playertype);
    if (playertype == BJ_PLAYER && cardidx == 1 && BJ_Games[playerid][BJ_Scores][BJ_PLAYER] < 21) {
        BJ_SetControlsActive(playerid, true);
    }
    return 0;
}

// Отображение курсора для нажатия текстдравов
forward BJ_SetControlsActive(playerid, bool:status);
public BJ_SetControlsActive(playerid, bool:status) {
    BJ_Games[playerid][BJ_IsControlsActive] = status;
    if (status) {
        SelectTextDraw(playerid, 0x8888DDFF);
    } else {
        CancelSelectTextDraw(playerid);
    }
}

// Переход хода от игрока к дилеру
stock BJ_SwitchTurn(playerid, e_BJ_PLAYERS:playertype) {
    BJ_Games[playerid][BJ_Turn] = playertype;
    if (playertype == BJ_DEALER) {
        BJ_SetControlsActive(playerid, false);
        SetTimerEx("BJ_FlipCard", BJ_DRAW_DELAY_MS, false, "iii", playerid, BJ_DEALER, 1);
    } else {
        BJ_ProcessDraw(playerid, playertype);
    }
    if (playertype > BJ_PLAYER) {
        BJ_Render_Score(playerid, BJ_PLAYER_SPLIT_FIRST);
        BJ_Render_Score(playerid, BJ_PLAYER_SPLIT_SECOND);
    }
}

// Действия при завершении хода игрока
stock BJ_EndPlayerTurn(playerid, e_BJ_PLAYERS:playertype) {
    switch (playertype) {
        case BJ_DEALER:
            BJ_EndGame(playerid);
        case BJ_PLAYER:
            BJ_SwitchTurn(playerid, BJ_DEALER);
        case BJ_PLAYER_SPLIT_FIRST:
            BJ_SwitchTurn(playerid, BJ_PLAYER_SPLIT_SECOND);
        case BJ_PLAYER_SPLIT_SECOND:
            BJ_SwitchTurn(playerid, BJ_DEALER);
        default:
            return 1;
    }
    return 0;
}

forward BJ_AddCard(playerid, e_BJ_PLAYERS:playertype);
public BJ_AddCard(playerid, e_BJ_PLAYERS:playertype) {
    switch (playertype) {
        case BJ_DEALER:
            BJ_Games[playerid][BJ_DealerHand][BJ_Games[playerid][BJ_CardsAmount][playertype]++] = random(CARD_STACK_SIZE);
        case BJ_PLAYER:
            BJ_Games[playerid][BJ_PlayerHand][BJ_Games[playerid][BJ_CardsAmount][playertype]++] = random(CARD_STACK_SIZE);
        case BJ_PLAYER_SPLIT_FIRST:
            BJ_Games[playerid][BJ_PlayerFirstHand][BJ_Games[playerid][BJ_CardsAmount][playertype]++] = random(CARD_STACK_SIZE);
        case BJ_PLAYER_SPLIT_SECOND:
            BJ_Games[playerid][BJ_PlayerSecondHand][BJ_Games[playerid][BJ_CardsAmount][playertype]++] = random(CARD_STACK_SIZE);
        default:
            return 1;
    }
    BJ_Render_Cards(playerid, playertype);
    BJ_UpdateSum(playerid, playertype);
    return 0;
}

// Добор карты
forward BJ_Hit(playerid, e_BJ_PLAYERS:playertype);
public BJ_Hit(playerid, e_BJ_PLAYERS:playertype) {
    BJ_AddCard(playerid, playertype);
    BJ_ProcessDraw(playerid, playertype);
    return 0;
}

// Завершение хода
forward BJ_Stay(playerid, e_BJ_PLAYERS:playertype);
public BJ_Stay(playerid, e_BJ_PLAYERS:playertype) {
    BJ_EndPlayerTurn(playerid, playertype);
    return 0;
}


// Удвоение ставки
forward BJ_Double(playerid, e_BJ_PLAYERS:playertype);
public BJ_Double(playerid, e_BJ_PLAYERS:playertype) {
    if (BJ_Games[playerid][BJ_CardsAmount][playertype] != 2) {
        SendClientMessage(playerid, 0xFFFFFFFF, "Удвоить ставку можно только на первом ходу!");
        return 1;
    }
    if (!AddPlayerMoney(playerid, -BJ_Games[playerid][BJ_Stakes][playertype - BJ_PLAYER])) {
        SendClientMessage(playerid, 0xFFFFFFFF, "Недостаточно средств!");
        return 1;
    }
    BJ_Games[playerid][BJ_Stakes][playertype - BJ_PLAYER] *= 2;
    BJ_AddCard(playerid, playertype);
    BJ_EndPlayerTurn(playerid, playertype);
    return 0;
}

// Разделение карт
forward BJ_Split(playerid);
public BJ_Split(playerid) {
    if (BJ_Games[playerid][BJ_CardsAmount][BJ_PLAYER] != 2) {
        SendClientMessage(playerid, 0xFFFFFFFF, "Разделить карты можно только на первом ходу!");
        return 1;
    }
    if (BJ_GetCardValue(BJ_Games[playerid][BJ_PlayerHand][0]) != BJ_GetCardValue(BJ_Games[playerid][BJ_PlayerHand][1])) {
        SendClientMessage(playerid, 0xFFFFFFFF, "Разделить можно только карты одинакового достоинства!");
        return 1;
    }
    if (!AddPlayerMoney(playerid, -BJ_Games[playerid][BJ_Stakes][0])) {
        SendClientMessage(playerid, 0xFFFFFFFF, "Недостаточно средств!");
        return 1;
    }
    BJ_Games[playerid][BJ_PlayerFirstHand][BJ_Games[playerid][BJ_CardsAmount][BJ_PLAYER_SPLIT_FIRST]++] = BJ_Games[playerid][BJ_PlayerHand][0];
    BJ_Games[playerid][BJ_PlayerSecondHand][BJ_Games[playerid][BJ_CardsAmount][BJ_PLAYER_SPLIT_SECOND]++] = BJ_Games[playerid][BJ_PlayerHand][1];
    // TODO: add em after delay? (through hit?)
    BJ_Games[playerid][BJ_PlayerFirstHand][BJ_Games[playerid][BJ_CardsAmount][BJ_PLAYER_SPLIT_FIRST]++] = random(CARD_STACK_SIZE);
    BJ_Games[playerid][BJ_PlayerSecondHand][BJ_Games[playerid][BJ_CardsAmount][BJ_PLAYER_SPLIT_SECOND]++] = random(CARD_STACK_SIZE);
    BJ_Games[playerid][BJ_Stakes][1] = BJ_Games[playerid][BJ_Stakes][0];
    BJ_Games[playerid][BJ_Stakes][2] = BJ_Games[playerid][BJ_Stakes][0];
    
    BJ_DeleteCards(playerid, BJ_PLAYER);
    if (BJ_Games[playerid][BJ_PlayerTextdraws][BJ_PLAYER_SCORE_TD] != INVALID_PLAYER_TEXT_ID) {
        PlayerTextDrawHide(playerid, BJ_Games[playerid][BJ_PlayerTextdraws][BJ_PLAYER_SCORE_TD]);
        PlayerTextDrawDestroy(playerid, BJ_Games[playerid][BJ_PlayerTextdraws][BJ_PLAYER_SCORE_TD]);
    }
    BJ_Games[playerid][BJ_Stakes][0] = 0;
    BJ_Games[playerid][BJ_CardsAmount][BJ_PLAYER] = 0;


    BJ_Render_Cards(playerid, BJ_PLAYER_SPLIT_FIRST);
    BJ_Render_Cards(playerid, BJ_PLAYER_SPLIT_SECOND);
    BJ_UpdateSum(playerid, BJ_PLAYER_SPLIT_FIRST);
    BJ_UpdateSum(playerid, BJ_PLAYER_SPLIT_SECOND);
    BJ_SwitchTurn(playerid, BJ_PLAYER_SPLIT_FIRST);
    // BJ_ProcessDraw(playerid, BJ_PLAYER_SPLIT_FIRST);
    return 0;
}

// Отступы по высоте для разных рук
new const Float:BJ_CARDS_Y_OFFSETS[4] = {100.0, 290.0, 290.0, 290.0};
// Отступы по ширине для разных рук (от центра)
new const Float:BJ_CARDS_X_OFFSETS[4] = {0.0, 0.0, -160.0, 160.0};

// Отрисовка отдельной карты по заданным координатам
stock PlayerText:BJ_DrawCard(playerid, Float:x, Float:y, const name[]) {
    new PlayerText:id = CreatePlayerTextDraw(playerid, x, y, name);
    PlayerTextDrawFont(playerid, id, TEXT_DRAW_FONT_SPRITE_DRAW);
    PlayerTextDrawLetterSize(playerid, id, 0.6, 2.0);
    PlayerTextDrawTextSize(playerid, id, 40.0, 60.0);
    PlayerTextDrawSetOutline(playerid, id, 1);
    PlayerTextDrawSetShadow(playerid, id, 0);
    PlayerTextDrawAlignment(playerid, id, TEXT_DRAW_ALIGN_RIGHT);
    PlayerTextDrawColour(playerid, id, -1);
    PlayerTextDrawBackgroundColour(playerid, id, 255);
    PlayerTextDrawBoxColour(playerid, id, 50);
    PlayerTextDrawUseBox(playerid, id, true);
    PlayerTextDrawSetProportional(playerid, id, true);
    PlayerTextDrawSetSelectable(playerid, id, false);
    PlayerTextDrawShow(playerid, id);
    return id;
}

// Удаление текстдравов карт у заданного игрока
stock BJ_DeleteCards(playerid, e_BJ_PLAYERS:playertype) {
    if (playertype == BJ_DEALER) {
        for (new i = 0; i < BJ_Games[playerid][BJ_CardsAmount][BJ_DEALER]; i++) {
            PlayerTextDrawHide(playerid, BJ_Games[playerid][BJ_DealerCardsTDs][i]);
            PlayerTextDrawDestroy(playerid, BJ_Games[playerid][BJ_DealerCardsTDs][i]);
        }
    } else if (playertype == BJ_PLAYER) {
        for (new i = 0; i < BJ_Games[playerid][BJ_CardsAmount][BJ_PLAYER]; i++) {
            PlayerTextDrawHide(playerid, BJ_Games[playerid][BJ_PlayerCardsTDs][i]);
            PlayerTextDrawDestroy(playerid, BJ_Games[playerid][BJ_PlayerCardsTDs][i]);
        }
    } else if (playertype == BJ_PLAYER_SPLIT_FIRST) {
        for (new i = 0; i < BJ_Games[playerid][BJ_CardsAmount][BJ_PLAYER_SPLIT_FIRST]; i++) {
            PlayerTextDrawHide(playerid, BJ_Games[playerid][BJ_PlayerFirstCardsTDs][i]);
            PlayerTextDrawDestroy(playerid, BJ_Games[playerid][BJ_PlayerFirstCardsTDs][i]);
        }
    } else if (playertype == BJ_PLAYER_SPLIT_SECOND) {
        for (new i = 0; i < BJ_Games[playerid][BJ_CardsAmount][BJ_PLAYER_SPLIT_SECOND]; i++) {
            PlayerTextDrawHide(playerid, BJ_Games[playerid][BJ_PlayerSecondCardsTDs][i]);
            PlayerTextDrawDestroy(playerid, BJ_Games[playerid][BJ_PlayerSecondCardsTDs][i]);
        }
    }
}

stock BJ_Render_Score(playerid, e_BJ_PLAYERS:playertype) {
    if (playertype > BJ_DEALER) {
        if (BJ_Games[playerid][BJ_PlayerTextdraws][e_BJ_TEXTDRAWS:playertype] != INVALID_PLAYER_TEXT_ID) {
            PlayerTextDrawHide(playerid, BJ_Games[playerid][BJ_PlayerTextdraws][e_BJ_TEXTDRAWS:playertype]);
            PlayerTextDrawDestroy(playerid, BJ_Games[playerid][BJ_PlayerTextdraws][e_BJ_TEXTDRAWS:playertype]);
        }
        // new PlayerText:tdid;
        new PlayerText:tdid = CreatePlayerTextDraw(playerid, 320.000000 + BJ_CARDS_X_OFFSETS[playertype], 255.000000, "You: %d", BJ_Games[playerid][BJ_Scores][playertype]);
        BJ_Games[playerid][BJ_PlayerTextdraws][e_BJ_TEXTDRAWS:playertype] = tdid;
        PlayerTextDrawFont(playerid, tdid, TEXT_DRAW_FONT_1);
        PlayerTextDrawLetterSize(playerid, tdid, 0.500000, 2.000000);
        PlayerTextDrawTextSize(playerid, tdid, 365.000000, 90.000000);
        PlayerTextDrawSetOutline(playerid, tdid, 1);
        PlayerTextDrawSetShadow(playerid, tdid, 0);
        PlayerTextDrawAlignment(playerid, tdid, TEXT_DRAW_ALIGN_CENTER);
        PlayerTextDrawColour(playerid, tdid, -1);
        PlayerTextDrawBackgroundColour(playerid, tdid, 255);
        PlayerTextDrawBoxColour(playerid, tdid, 0xAAAAAAAA);
        PlayerTextDrawUseBox(playerid, tdid, playertype == BJ_Games[playerid][BJ_Turn]);
        PlayerTextDrawSetProportional(playerid, tdid, true);
        PlayerTextDrawSetSelectable(playerid, tdid, false);
        PlayerTextDrawShow(playerid, tdid);
    }
}

// Отрисовка карт у заданного игрока
stock BJ_Render_Cards(playerid, e_BJ_PLAYERS:playertype) {
    BJ_DeleteCards(playerid, playertype);
    BJ_Render_Score(playerid, playertype);
    new amt = BJ_Games[playerid][BJ_CardsAmount][playertype];
    new x_offset = 20 + 25 * (amt - 1);
    new PlayerText:tdid;
    for (new j = 0; j < amt; j++) {
        new card_name[MAX_CARD_NAME_LEN], card_id;
        switch (playertype) {
            case BJ_DEALER:
                card_id = BJ_Games[playerid][BJ_DealerHand][j];
            case BJ_PLAYER:
                card_id = BJ_Games[playerid][BJ_PlayerHand][j];
            case BJ_PLAYER_SPLIT_FIRST:
                card_id = BJ_Games[playerid][BJ_PlayerFirstHand][j];
            case BJ_PLAYER_SPLIT_SECOND:
                card_id = BJ_Games[playerid][BJ_PlayerSecondHand][j];
            default:
                printf("WTF: Player type %d", playertype);
        }
        GetPlayingCardSpriteName(card_name, card_id % 13, card_id / 13);
        tdid = BJ_DrawCard(playerid, 320.0 - x_offset + BJ_CARDS_X_OFFSETS[playertype], BJ_CARDS_Y_OFFSETS[playertype], card_name);
        // printf("card tdid: %d", tdid);
        switch (playertype) {
            case BJ_DEALER:
                BJ_Games[playerid][BJ_DealerCardsTDs][j] = tdid;
            case BJ_PLAYER:
                BJ_Games[playerid][BJ_PlayerCardsTDs][j] = tdid;
            case BJ_PLAYER_SPLIT_FIRST:
                BJ_Games[playerid][BJ_PlayerFirstCardsTDs][j] = tdid;
            case BJ_PLAYER_SPLIT_SECOND:
                BJ_Games[playerid][BJ_PlayerSecondCardsTDs][j] = tdid;
            default:
                printf("WTF: Player type %d", playertype);
        }
        x_offset -= 50;
    }
    return 0;
}

// Отрисовка контролов 
stock BJ_DrawPlayerButtons(playerid) {
    new PlayerText:tdid;

    tdid = CreatePlayerTextDraw(playerid, 320.000000, 65.000000, "Dealer: 0");
    BJ_Games[playerid][BJ_PlayerTextdraws][BJ_DEALER_SCORE_TD] = tdid;
    PlayerTextDrawFont(playerid, tdid, TEXT_DRAW_FONT_1);
    PlayerTextDrawLetterSize(playerid, tdid, 0.500000, 2.000000);
    PlayerTextDrawTextSize(playerid, tdid, 365.000000, 103.000000);
    PlayerTextDrawSetOutline(playerid, tdid, 1);
    PlayerTextDrawSetShadow(playerid, tdid, 0);
    PlayerTextDrawAlignment(playerid, tdid, TEXT_DRAW_ALIGN_CENTER);
    PlayerTextDrawColour(playerid, tdid, -1);
    PlayerTextDrawBackgroundColour(playerid, tdid, 255);
    PlayerTextDrawBoxColour(playerid, tdid, 16777010);
    PlayerTextDrawUseBox(playerid, tdid, false);
    PlayerTextDrawSetProportional(playerid, tdid, true);
    PlayerTextDrawSetSelectable(playerid, tdid, false);
    PlayerTextDrawShow(playerid, tdid);

    tdid = CreatePlayerTextDraw(playerid, 170.0, 390.0, "HIT");
    BJ_Games[playerid][BJ_PlayerTextdraws][BJ_HIT_TEXTDRAW] = tdid;
    PlayerTextDrawFont(playerid, tdid, TEXT_DRAW_FONT_2);
    PlayerTextDrawLetterSize(playerid, tdid, 0.5, 2.0);
    PlayerTextDrawTextSize(playerid, tdid, 47.5, 80.0);
    PlayerTextDrawSetOutline(playerid, tdid, 1);
    PlayerTextDrawSetShadow(playerid, tdid, 0);
    PlayerTextDrawAlignment(playerid, tdid, TEXT_DRAW_ALIGN_CENTER);
    PlayerTextDrawColour(playerid, tdid, -1);
    PlayerTextDrawBackgroundColour(playerid, tdid, 255);
    PlayerTextDrawBoxColour(playerid, tdid, 135);
    PlayerTextDrawUseBox(playerid, tdid, true);
    PlayerTextDrawSetProportional(playerid, tdid, true);
    PlayerTextDrawSetSelectable(playerid, tdid, true);
    PlayerTextDrawShow(playerid, tdid);

    tdid = CreatePlayerTextDraw(playerid, 270.000000, 390.000000, "DOUBLE");
    BJ_Games[playerid][BJ_PlayerTextdraws][BJ_DOUBLE_TEXTDRAW] = tdid;
    PlayerTextDrawFont(playerid, tdid, TEXT_DRAW_FONT_2);
    PlayerTextDrawLetterSize(playerid, tdid, 0.500000, 2.000000);
    PlayerTextDrawTextSize(playerid, tdid, 47.500000, 80.000000);
    PlayerTextDrawSetOutline(playerid, tdid, 1);
    PlayerTextDrawSetShadow(playerid, tdid, 0);
    PlayerTextDrawAlignment(playerid, tdid, TEXT_DRAW_ALIGN_CENTER);
    PlayerTextDrawColour(playerid, tdid, -1);
    PlayerTextDrawBackgroundColour(playerid, tdid, 255);
    PlayerTextDrawBoxColour(playerid, tdid, 135);
    PlayerTextDrawUseBox(playerid, tdid, true);
    PlayerTextDrawSetProportional(playerid, tdid, true);
    PlayerTextDrawSetSelectable(playerid, tdid, true);
    PlayerTextDrawShow(playerid, tdid);

    tdid = CreatePlayerTextDraw(playerid, 370.000000, 390.000000, "STAY");
    BJ_Games[playerid][BJ_PlayerTextdraws][BJ_STAY_TEXTDRAW] = tdid;
    PlayerTextDrawFont(playerid, tdid, TEXT_DRAW_FONT_2);
    PlayerTextDrawLetterSize(playerid, tdid, 0.500000, 2.000000);
    PlayerTextDrawTextSize(playerid, tdid, 47.500000, 80.000000);
    PlayerTextDrawSetOutline(playerid, tdid, 1);
    PlayerTextDrawSetShadow(playerid, tdid, 0);
    PlayerTextDrawAlignment(playerid, tdid, TEXT_DRAW_ALIGN_CENTER);
    PlayerTextDrawColour(playerid, tdid, -1);
    PlayerTextDrawBackgroundColour(playerid, tdid, 255);
    PlayerTextDrawBoxColour(playerid, tdid, 135);
    PlayerTextDrawUseBox(playerid, tdid, true);
    PlayerTextDrawSetProportional(playerid, tdid, true);
    PlayerTextDrawSetSelectable(playerid, tdid, true);
    PlayerTextDrawShow(playerid, tdid);

    tdid = CreatePlayerTextDraw(playerid, 470.000000, 390.000000, "SPLIT");
    BJ_Games[playerid][BJ_PlayerTextdraws][BJ_SPLIT_TEXTDRAW] = tdid;
    PlayerTextDrawFont(playerid, tdid, TEXT_DRAW_FONT_2);
    PlayerTextDrawLetterSize(playerid, tdid, 0.500000, 2.000000);
    PlayerTextDrawTextSize(playerid, tdid, 47.500000, 80.000000);
    PlayerTextDrawSetOutline(playerid, tdid, 1);
    PlayerTextDrawSetShadow(playerid, tdid, 0);
    PlayerTextDrawAlignment(playerid, tdid, TEXT_DRAW_ALIGN_CENTER);
    PlayerTextDrawColour(playerid, tdid, -1);
    PlayerTextDrawBackgroundColour(playerid, tdid, 255);
    PlayerTextDrawBoxColour(playerid, tdid, 135);
    PlayerTextDrawUseBox(playerid, tdid, true);
    PlayerTextDrawSetProportional(playerid, tdid, true);
    PlayerTextDrawSetSelectable(playerid, tdid, true);
    PlayerTextDrawShow(playerid, tdid);
    return 1;
}

cmd:bj(playerid, args[]) {
    if (BJ_Games[playerid][BJ_IsActive]) {
        SendClientMessage(playerid, 0xFFFFFFFF, "Игра уже идет!");
        return 0;
    }
    new stake;
    sscanf(args, "i", stake);
    if (stake < BJ_MIN_STAKE) {
        SendClientMessage(playerid, 0xFFFFFFFF, "Ставка не может быть меньше %d!", BJ_MIN_STAKE);
        return 0;
    }
    if (stake > BJ_MAX_STAKE) {
        SendClientMessage(playerid, 0xFFFFFFFF, "Ставка не может быть больше %d!", BJ_MAX_STAKE);
        return 0;
    }
    if (!AddPlayerMoney(playerid, -stake)) {
        SendClientMessage(playerid, 0xFFFFFFFF, "Недостаточно средств!");
        return 0;
    }
    BJ_StartGame(playerid, stake);
    return 1;
}

hook OnPlayerClickPlayerTD(playerid, PlayerText:playertextid) {
    if (!BJ_Games[playerid][BJ_IsControlsActive]) {
        return 1;
    }
    if (playertextid == BJ_Games[playerid][BJ_PlayerTextdraws][BJ_HIT_TEXTDRAW]) {
        BJ_Hit(playerid, BJ_Games[playerid][BJ_Turn]);
    } else if (playertextid == BJ_Games[playerid][BJ_PlayerTextdraws][BJ_STAY_TEXTDRAW]) {
        BJ_Stay(playerid, BJ_Games[playerid][BJ_Turn]);
    } else if (playertextid == BJ_Games[playerid][BJ_PlayerTextdraws][BJ_DOUBLE_TEXTDRAW]) {
        BJ_Double(playerid, BJ_Games[playerid][BJ_Turn]);
    } else if (playertextid == BJ_Games[playerid][BJ_PlayerTextdraws][BJ_SPLIT_TEXTDRAW]) {
        BJ_Split(playerid);
    }
    return 0;
}