kj
var wasCleaned = false;
var person = null;
var partnerName;
var yourFig = "X";
var idUpdListOfCreators;
var idUpdListOfPlayers;
var idWaitingPlayer;
var idUpdPlayField;
var size;
var gameNotStarted = true;
var myTurn;
var gameField = null;

$(document).ready(function() {
    hideAllSteps();
    $("#registrationStep").slideDown();

    $("#signInButton").click(signInOnClick);

    $("#createFieldButton").click(createFieldOnClick);
    $("#createButton").click(createOnClick);
    $("#cancelButton").click(cancelOnClick);

    $("#joinButton").click(joinOnClick);
    $("#updListOfCreators").on("click", updListOfCreater);

    $("#updReadyButton").on("click", updListOfReadyPlayers);
});

function cancelOnClick() {
    var request = "name=" + person;
    $.ajax({
        type: 'POST',
        url: "cancel_create_field",
        data: request,
        success: function(data) {
            $("#createFieldStep *").attr("disabled", false);
        }
    });
}

function createOnClick() {
    $("#createFieldStep *").attr("disabled", true);
    $("#cancelButton").attr("disabled", false);
    $("#updReadyButton").attr("disabled", false);
    
    var request = "name=" + person;
    request += "&size=" + size;
    $.ajax({
        type: 'POST',
        url: "create_field",
        data: request,
        success: function(data) {
            updListOfReadyPlayers();
            idUpdListOfPlayers = setInterval(updListOfReadyPlayers, 4000);
        }
    });
}

function createMessageFromJson(data, begin, end) {
    var array = eval(data);
    var message = "";
    for (var i = 0; i < array.length; ++i) {
       message += begin + array[i] + end;
    }
    return message;
}

function updListOfReadyPlayers() {
    var request = "name=" + person;
    $.ajax({
        type: 'POST',
        url: "get_ready_players",
        success: function(data) {
            var message = createMessageFromJson(data, "<tr><td>", "</td></tr>");
            $("#listOfReadyPlayers").html(message);
            $("#listOfReadyPlayers td").on("click", playWith);
        },
        data: request
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

function hideAllSteps() {
    $("#registrationStep").hide();
    $("#myNameBlock").hide();
    $("#partnerNameBlock").hide();
    $("#intermediateStep").hide();
    $("#createFieldStep").hide();
    $("#playFieldStep").hide();
    $("#joinStep").hide();
}

function joinOnClick() {
    hideAllSteps();
    $("#joinStep").slideDown();
    updListOfCreater()
    idUpdListOfCreators = setInterval(updListOfCreater, 4000);
}

function updListOfCreater() {
    $.ajax({
        type: 'POST',
        url: 'get_creators',
        data: "",
        success: function(data) {
            var message = createMessageFromJson(data, "<tr><td>", "</td></tr>");
            $('#listOfCreator').html(message);
            $('#listOfCreator td').on("click", joinWith);
        },
        error: function(xhr, status) {
            alert(xhr.status);
            alert(status);
        }
 
    });
}

// not implemnted block elements
function joinWith() {
    partnerName = $(this).text();
    var request = "name=" + person;
    request += "&with=" + partnerName;
    $.ajax({
        type: 'POST',
        url: 'join_with',
        data: request,
        success: function(data) {
            idWaitingPlayer = setInterval(checkIfAccept, 1000);
        }
    });
}

function checkIfAccept() {
    var request = "name=" + person;
    $.ajax({
        type: 'POST',
        url: 'check_if_accept',
        data: request,
        success: function(data) {
            clearInterval(idWaitingPlayer);
            clearInterval(idUpdListOfCreators);
            connectWithCreator(data);
        }
    });
}

function showPlayField() {
    $("#myNameBlock").slideDown();
    $("#playFieldStep").slideDown();
    $("#partnerNameBlock").slideDown();
    $("#partnerName").html(partnerName);
    gameNotStarted = false;
    idUpdPlayField = setInterval(updPlayField, 1000);
}

function updPlayField() {
    var request = "name=" + person;
    $.ajax({
        type: 'POST',
        url: 'get_play_field',
        data: request,
        success: function(data) {
            gameField = eval(data);
            changeGameFieldWithSize();
        }
    });
    updTurn();
}

function updTurn() {
    var request = "name=" + person;
    $.ajax({
        type: 'POST',
        url: 'get_turn',
        data: request,
        success: function(data) {
            $("#myTurn").html("Its your turn");
            myTurn = true;
            clearInterval(idUpdPlayField);
        },
        error: function(xhr, status) {
            if (xhr.status == 400) {
                $("#myTurn").html("Its " + partnerName + "'s turn, wait...");
                if (myTurn) {
                    idUpdPlayField = setInterval(updPlayField, 1000);
                }
                myTurn = false;
            }
        }
    });
}

function connectWithCreator(data) {
    yourFig = "O";
    myTurn = false;
    var response = data.split("&");
    var response_map = {};
    for (var i = 0; i < response.length; ++i) {
        var temp = response[i].split("=");
        response_map[temp[0]] = temp[1];
    }
    size = response_map["size"];
    hideAllSteps();
    showPlayField();
    changeGameFieldWithSize();
}

function createFieldOnClick() {
    hideAllSteps();
    $("#createFieldStep").slideDown();
    $("#myNameBlock").slideDown();
    selector = $("#sizeField")
    initSelectorOfSizeField(selector);
    selector.on("change", changeGameField);
    selector.change()
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

function signInOnClick() {
    person = $("#name").val();
    if (person) { //person !=null, != ""
        $.ajax({
            type: 'POST',
            url: "new_player",
            data: 'name=' + person,
            success: function(data) {
                $("#myName").html(person);
                hideAllSteps();
                $("#myNameBlock").slideDown();
                $("#intermediateStep").slideDown();

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

function changeGameField() {
    size = $(this).val();
    changeGameFieldWithSize();
}

function changeGameFieldWithSize() {
    var resultTable = "";
    for (var i = 0; i < size; i++) {
        resultTable += "<tr>";
        for (var j = 0; j < size; j++) {
            var fig = gameField == null ? ' ' : gameField[i][j];
            resultTable += "<td>" + fig + "</td>";
        }
        resultTable += "</tr>";
    }

    $(".gameField").html(resultTable);

    $(".gameField td").click(function() {     
        var text = $(this).html();
        if (gameNotStarted) {
            if (text == "") {
                $(this).html(yourFig);
            } else {
                $(this).html("");
            }
            return;
        }

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
            idUpdPlayField = setInterval(updPlayField, 1000);
        }
    });
}
