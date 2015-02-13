var wasCleaned = false;
var person = null;
var partner;
var yourFig = "X";
var timoutIdWaitingCreator;
var size;

$(document).ready(function() {
    $("#myNameBlock").hide();
    $("#intermediateStep").hide();
    $("#createFieldStep").hide();
    $("#playFieldStep").hide();
    $("#joinStep").hide();

    $("#signInButton").click(signInOnClick);

    $("#createFieldButton").click(createFieldOnClick);
    $("#createButton").click(createOnClick);
    $("#cancelButton").click(cancelOnClick);

    $("#joinButton").click(joinOnClick);
    $("#updListOfCreators").on("click", updListOfCreater);

    $("#updReadyButton").on("click", updListOfReadyPlayers);

    setInterval(updCurPlayers, 1000); 
});

function cancelOnClick() {
    $("#createFieldStep *").attr("disabled", false);
}

function createOnClick() {
    $("#createFieldStep *").attr("disabled", true);
    $("#cancelButton").attr("disabled", false);
    $("#updateReadyPlayers").attr("disabled", false);
    
    var request = "name=" + person;
    request += "&size=" + size;
    $.ajax({
        type: 'POST',
        url: "create_field",
        data: request
    });

}

function updListOfReadyPlayers() {
    var request = "name=" + person;
    $.ajax({
        type: 'POST',
        url: "get_ready_players",
        success: function(data) {
            $("#listOfReadyPlayers").html(data);
            $("#listOfReadyPlayers td").on("click", playWith);
        },
        data: request
    });
}

function playWith() {
    alert("playWith: " + $(this).val());
}

function joinOnClick() {
    $("#intermediateStep").slideUp();
    $("#joinStep").show();
    updListOfCreater();
}

function updListOfCreater() {
    $.ajax({
        type: 'POST',
        url: 'get_creators',
        success: function(data) {
            $('#listOfCreator').html(data)
            $('#listOfCreator td').on("click", joinWith);
        }
    });
}

function joinWith() {
    var request = "name=" + person;
    request += "&with=" + $(this).text();
    $.ajax({
        type: 'POST',
        url: 'join_with',
        data: request,
        success: function(data) {
            yourFig = "O";
            var response = data.split("&");
            var response_map = {};
            for (var i = 0; i < response.length; ++i) {
                var temp = response[i].split("=");
                response_map[temp[0]] = temp[1];
            }
            size = response_map["size"];
            $("#playFieldStep").show();
            $("#joinStep").slideUp();
            changeGameFieldWithSize();
        }
    });
}

function createFieldOnClick() {
    $("#intermediateStep").slideUp();
    $("#createFieldStep").show();
    selector = $("#sizeField")
    initSelectorOfSizeField(selector);
    selector.on("change", changeGameField);
    selector.change()
}

function updCurPlayers(){
    $.ajax({
      url: 'get_all_players',
      success: function(data) {
        $('#listOfPlayers').html(data)
      }
    });
}

function pageCleanup() {
    if (wasCleaned || !person) return;
    $.ajax({
        type: 'POST',
        url: "leave_game",
        async: false,
        data: 'name=' + person,
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
                $("#myNameBlock").show();
                $("#intermediateStep").show();
                $("#registrationStep").slideUp();

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
    var textTd = "";
    var resultTable = "";
    for (var i = 0; i < size; i++) {
       textTd += "<td></td>"; 
    }

    for (var i = 0; i < size; i++) {
        resultTable += "<tr>" + textTd + "</tr>";
    }

    $(".gameField").html(resultTable);

    $(".gameField td").click(function() {     
        if ($(this).html()) {
            $(this).html("");
        } else {
            $(this).html(yourFig);
        }
        var column_num = parseInt( $(this).index() ) + 1;
        var row_num = parseInt( $(this).parent().index() )+1;    
    });
}
