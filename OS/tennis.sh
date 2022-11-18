#!/bin/bash
#Omer Arad 314096389

game_on=true

printBoard () {
    echo " Player 1: ${POINTS_P1}         Player 2: ${POINTS_P2} "
    echo " --------------------------------- "
    echo " |       |       #       |       | "
    echo " |       |       #       |       | "
    echo "$BALL_PRINT"
    echo " |       |       #       |       | "
    echo " |       |       #       |       | "
    echo " --------------------------------- "
}

run () {
    init
    printBoard
    while $game_on
    do
        playP1
        printBoard
        echo -e "       Player 1 played: ${num1}\n       Player 2 played: ${num2}\n\n"
        checkWinner
    done
}

init () {
    BALL_PRINT=" |       |       O       |       | "
    BALL_PLACE=0
    POINTS_P1=50
    POINTS_P2=50
    NON_VALID="NOT A VALID MOVE !"
}

playP1 () {
    echo "PLAYER 1 PICK A NUMBER: "
    read -s num1
    if [[ ! $num1 =~ ^[0-9]+$ || $num1 -lt 0 || $num1 -gt POINTS_P1 ]]
    then
        echo "$NON_VALID"
        playP1
    else
        POINTS_P1=$(( POINTS_P1 - num1 ))
        playP2
    fi
}

playP2 () {
    echo "PLAYER 2 PICK A NUMBER: "
    read -s num2
    if [[ ! $num2 =~ ^[0-9]+$ || $num2 -lt 0 || $num2 -gt POINTS_P2 ]]
    then
        echo "$NON_VALID"
        playP2
    else
        POINTS_P2=$(( POINTS_P2 - num2 ))
        move
    fi
}

move () {
    if [ $num1 -gt $num2 ]
    then
        if [[ BALL_PLACE -lt 0 ]]
        then
	    BALL_PLACE=1
        else
	    ((BALL_PLACE++))
        fi
    elif [ $num2 -gt $num1 ]
    then
        if [[ BALL_PLACE -gt 0 ]]
        then
	    BALL_PLACE=-1
        else
	    ((BALL_PLACE--))
        fi
    fi
    printMove
}

printMove () {
    case $BALL_PLACE in
        -3)
        BALL_PRINT="O|       |       #       |       | "
        ;;
        -2)
        BALL_PRINT=" |   O   |       #       |       | "
        ;;
        -1)
        BALL_PRINT=" |       |   O   #       |       | "
        ;;
        0)
        BALL_PRINT=" |       |       O       |       | "
        ;;
        1)
        BALL_PRINT=" |       |       #   O   |       | "
        ;;
        2)
        BALL_PRINT=" |       |       #       |   O   | "
        ;;
        3)
        BALL_PRINT=" |       |       #       |       |O"
        ;;
    esac
}

checkWinner () {
    if [[ BALL_PLACE -eq 3 ]]
    then
	game_on=false
	echo "PLAYER 1 WINS !"
    elif [[ BALL_PLACE -eq -3 ]]
    then
	game_on=false
	echo "PLAYER 2 WINS !"
    elif [[ POINTS_P1 -eq 0 && POINTS_P2 -ne 0 ]]
    then
	game_on=false
	echo "PLAYER 2 WINS !"
    elif [[ POINTS_P2 -eq 0 && POINTS_P1 -ne 0 ]]
    then
	game_on=false
	echo "PLAYER 1 WINS !"
    elif [[ POINTS_P1 -eq 0 && POINTS_P2 -eq 0 ]]
    then
	game_on=false

	if [[ BALL_PLACE -gt 0 ]]
	then
	    echo "PLAYER 1 WINS !"
	elif [[ BALL_PLACE -lt 0 ]]
	then
	    echo "PLAYER 2 WINS !"
	else
	    echo "IT'S A DRAW !"
	fi
    fi
    if [ $game_on == false ]; then return; fi
}

run
