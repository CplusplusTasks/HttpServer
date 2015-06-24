var wasCleaned = false;
var person = null;
var partnerName;
var yourFig = "X";
var idUpdListOfCreators;
var idUpdListOfPlayers;
var idWaitingPlayer;
var idUpdPlayField;
var fieldSize;
var gameNotStarted = true;
var myTurn;
var lastStep = null;
var resultTable = null;
var iWin = null;
var numWins = 0;
var numLose = 0;

$(document).ready(function() {
    hideAllSteps();
    $("#myNameBlock").hide();
    $("#readyPlayers").hide();
    $("#registrationStep").slideDown();

    $("#signInButton").click(signInOnClick);
});

function signInOnClick() {
    person = $("#name").val();
    var request = "name=" + person;
    if (person) { //person !=null, != ""
        $.ajax({
            type: 'POST',
            url: "new_player",
            data: request,
            success: function(data) {
                $("#myName").html(person);
                hideAllSteps();
                $("#intermediateStep").slideDown();

                $("#createFieldButton").click(createFieldOnClick);
                $("#joinButton").click(joinOnClick);

                $(window).on('beforeunload', pageCleanup); // only for Chrome
                $(window).on("unload", pageCleanup); // for other browsers
            },
            error: function(xhr, status) {
                if (xhr.status == 400) {
                    alert("Someone already has that nickname.\n Try another?")
                }
            }
        });
    }
}

function createFieldOnClick() {
    hideAllSteps();
    $("#createFieldStep").slideDown();
    selector = $("#sizeField");
    initSelectorOfSizeField(selector);
    selector.on("change", changeGameField);
    selector.change();

    $("#createButton").click(createOnClick);
    $("#cancelCreateButton").click(cancelCreateOnClick);
}

function createOnClick() {
    $("#createFieldStep *").attr("disabled", true);
    $("#cancelCreateButton").attr("disabled", false);
    $("#readyPlayers").slideDown();
    $("#listOfReadyPlayers").html("<tr><td>wait please...</td></tr>");
    
    var request = "name=" + person;
    request += "&size=" + fieldSize;
    $.ajax({
        type: 'POST',
        url: "create_field",
        data: request,
        success: function(data) {
            updListOfReadyPlayers();
            idUpdListOfPlayers = setInterval(updListOfReadyPlayers, 2000);
        }
    } );
}

function cancelCreateOnClick() {
    var request = "name=" + person;
    $("#listOfReadyPlayers").html("");
    $("#readyPlayers").slideUp();
    $.ajax({
        type: 'POST',
        url: "cancel_create_field",
        data: request,
        success: function(data) {
            $("#createFieldStep *").attr("disabled", false);
        }
    });
}

function updListOfReadyPlayers()  {
    getGameState(function(data) {
        var readyPlayers = data.readyPlayers;
        var message = createMessageFromJson(readyPlayers, "<tr><td>", "</td></tr>");
        $("#listOfReadyPlayers").html(message);
        $("#listOfReadyPlayers td").on("click", playWith);
    });
}

function playWith() {
    partnerName = $(this).text();
    var request = "name=" + person;
    request += "&with=" + partnerName;
    $.ajax({
        type: 'POST',
        url: "start_play",
        data: request,
        success: function(data) {
            clearInterval(idUpdListOfPlayers);
            clearInterval(idUpdPlayField);
            hideAllSteps();
            showPlayField();
            myTurn = true;
        }
    });
}

function joinOnClick() {
    hideAllSteps();
    $("#joinStep").slideDown();
    updListOfCreators()
    idUpdListOfCreators = setInterval(updListOfCreators, 2000);
}

function updListOfCreators() {
    getGameState(function(data) {
        var creators = data.creators;
        var message = createMessageFromJson(creators, "<tr><td>", "</td></tr>");
        $('#listOfCreator').html(message);
        $('#listOfCreator td').on("click", joinWith);
    });
}

// not implemnted block elements
function joinWith() {
    $("#joinStep").append("<p>wait please...</p>");
    clearInterval(idUpdListOfCreators);
    $("#joinStep *").attr("disabled", true);
    $('#listOfCreator td').attr('onclick','').unbind('click');
    partnerName = $(this).text();
    var request = "name=" + person;
    request += "&with=" + partnerName;
    $.ajax({
        type: 'POST',
        url: 'join_with',
        data: request,
        success: function(data) {
            idWaitingPlayer = setInterval(checkIfAccept, 200);
        }
    });
}

function checkIfAccept() {
    getGameState(function(data) {
        if (!data.fieldSize) return;
        clearInterval(idWaitingPlayer);
        clearInterval(idUpdListOfCreators);
        connectWithCreator(data);
    });
}

function showPlayField() {
    $("#playFieldStep").slideDown();
    $("#partnerNameBlock").slideDown();
    $("#partnerName").html(partnerName);
    $("#playAgainButton").click(playAgain);

    gameNotStarted = false;
    idUpdPlayField = setInterval(updPlayField, 200);
}

function playAgain() {
    if (!gameNotStarted) return;
    resultTable = null;
    $.ajax({
        type: 'POST',
        url: "play_again",
        data: "name=" + person,
        success: function(data) {
            gameNotStarted = false;
            changeGameFieldWithSize();
            updPlayField();
            idUpdPlayField = setInterval(updPlayField, 200);
        }
    });
}

function updPlayField() {
    getGameState(function(data) {
        if (gameNotStarted) return;
        lastStep = data.lastStep;
        myTurn = data.myTurn;
        yourFig = data.myFig;
        iWin = data.win;
        if (iWin != undefined) {
            changeGameFieldWithSize();
            if (iWin == true) {
                alert("You win!");
                numWins++;
            } else if (iWin == false) {
                alert("You lose!");
                numLose++;
            }
            $("#myNumWins").html(numWins);
            $("#partnerNumWins").html(numLose);
            clearInterval(idUpdPlayField);
            gameNotStarted = true;
            myTurn = false;
            return;
        }
        if (myTurn) {
            $("#myTurn").html("Its your turn");
            clearInterval(idUpdPlayField);
        } else {
            $("#myTurn").html("Its " + partnerName + "'s turn, wait...");
        }
        changeGameFieldWithSize();
    });
}

function connectWithCreator(data) {
    myTurn = data.myTurn;
    yourFig = data.myFig;
    fieldSize = data.fieldSize;
    resultTable = null;
    hideAllSteps();
    showPlayField();
    changeGameFieldWithSize();
}

function changeGameField() {
    fieldSize = $(this).val();
    resultTable = null;
    changeGameFieldWithSize();
}

// "allPlayers: ";
// "myTurn: ";
// "fieldSize: "
// "lastStep: {row, column, fig},";
// "readyPlayers: ";
// "creators:";
function changeGameFieldWithSize() {
    if (lastStep) {
        var element = $("#gameField tr:nth-child(" + (lastStep.row + 1) + ")>td:nth-child(" + (lastStep.column + 1) + ")");
        element.html(lastStep.fig).css("color", "blue");
        lastStep = null;
    } else if (!resultTable) {
        resultTable = "";
        for (var i = 0; i < fieldSize; ++i) {
            resultTable += "<tr>";
            for (var j = 0; j < fieldSize; ++j) {
                resultTable += "<td> </td>";
            }
            resultTable += "</tr>";
        }
        $(".gameField").html(resultTable);
    }

    $(".gameField td").click(function() {     
        var text = $(this).html();
        if (gameNotStarted) return;

        if (myTurn) {
            if (text && text != " ") return;
            var column = parseInt( $(this).index() );
            var row = parseInt( $(this).parent().index() );    
            $(this).html(yourFig);

            var request = "name=" + person;
            request += "&row=" + row;
            request += "&column=" + column;
            $.ajax({
                type: 'POST',
                url: "put_fig",
                data: request,
            });
            myTurn = false;
            idUpdPlayField = setInterval(updPlayField, 200);
            $("#gameField *").css("color", "#808080");
        }
    });
}

function hideAllSteps() {
    $("#myNameBlock").slideDown();

    $("#registrationStep").hide();
    $("#partnerNameBlock").hide();
    $("#intermediateStep").hide();
    $("#createFieldStep").hide();
    $("#playFieldStep").hide();
    $("#joinStep").hide();
}

function getGameState(callbackOnSuccess) {
    request = "name=" + person
    $.ajax({
        type: 'POST',
        url: "get_game_state",
        data: request,
        success: callbackOnSuccess,
        error: function(xhr, status) {
            //alert("ERRROR");
            //alert(xhr.statusText);
            //alert(xhr.responseText);
            //alert(xhr.status);
            //alert(thrownError);
        }
    });
}

function createMessageFromJson(data, begin, end) {
    var array = data;
    var message = "";
    for (var i = 0; i < array.length; ++i) {
       message += begin + array[i] + end;
    }
    return message;
}

function initSelectorOfSizeField(selector) {
    var arr = [];
    for (var i = 2; i < 50; i++) {
        arr.push({val : i, text : i});
    }
    $(arr).each(function() {
        if (this.val == 3) {
             selector.append($("<option selected='selected'>").attr('value',this.val).text(this.text));
        } else {
             selector.append($("<option>").attr('value',this.val).text(this.text));
        }
    });
} 

function pageCleanup() {
    if (wasCleaned || !person) return;
    var request = "name=" + person;
    request += "&with=" + partnerName;
    $.ajax({
        type: 'POST',
        url: "leave_game",
        async: false,
        data: request,
        success: function() {
            wasCleaned = true;
        }

    });
    return;
}

