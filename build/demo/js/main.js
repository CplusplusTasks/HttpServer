var wasCleaned = false;
var person = null;
$(document).ready(function(){
    $("#mainTable").hide();
    $("#playFieldStep").hide();
    $("#nextStep").click(function() {
        person = $("#name").val();
        if (person != null && person != "") {
            $.ajax({
                type: 'POST',
                url: "new_player",
                data: 'name=' + person,
                success: function(data) {
                    $("#createField").show();
                    $("#registrationStep").slideUp();
                    $("#myName").html(person);
                },
                error: function(xhr, status) {
                    if (xhr.status == 400) {
                        alert("Someone already has that nickname.\n Try another?")
                    }
                }
            });
        }
    });

    $("#")

    var sel = $("#sizeField")
    function initSelection() {
        var arr = [];
        for (var i = 2; i < 50; i++) {
            arr.push({val : i, text : i});
        }
        $(arr).each(function() {
            if (this.val == 3) {
                 sel.append($("<option selected='selected'>").attr('value',this.val).text(this.text));
            } else {
                 sel.append($("<option>").attr('value',this.val).text(this.text));
            }
        });
    }

    initSelection();

    sel.click(function() {
        var size = $(this).val();
        var textTd = "";
        for (var i = 0; i < size; i++) {
           textTd += "<td></td>"; 
        }
        var resultTable = "";
        for (var i = 0; i < size; i++) {
            resultTable += "<tr>" + textTd + "</tr>";
        }

        $("#gameField").html(resultTable);

        $("#gameField td").click(function() {     
     
            $(this).html("X");
            var column_num = parseInt( $(this).index() ) + 1;
            var row_num = parseInt( $(this).parent().index() )+1;    
     
            $("#result").html( "Row_num =" + row_num + "  ,  Rolumn_num ="+ column_num );   
        });
    });
    sel.click();

    updCurPlayers();
    setInterval(updCurPlayers, 1000); 

    $(window).on('beforeunload', pageCleanup);
    $(window).on("unload", pageCleanup);

});

function updCurPlayers(){
    $.ajax({
      url: 'get_players',
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
