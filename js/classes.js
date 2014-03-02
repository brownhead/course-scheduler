var Classes = Classes || {};

Classes.input_id = 1;

Classes.init = function() {
    var $formgroup = $("#classes-submit")

    for (var i = 0; i < 5; ++i) {
        var $input = $(document.createElement("input"));
        $input.attr("type", "text")
              .attr("class", "form-control")
              .attr("id", "class" + Classes.input_id.toString())
              .attr("name", "class" + Classes.input_id.toString())
              .attr("placeholder", "Enter Class");
        ++Classes.input_id;
        $formgroup.append($input);
    }

    $("#schedule-form").submit(function(e) {
        var $encoding = $(document.createElement("input"));
        $encoding.attr("type", "hidden")
                 .attr("name", "encoding")
                 .attr("value", Table.encoded());
        $formgroup.append($encoding);
        e.preventDefault();

        var posting = $.post("http://localhost:8080/schedule", $("#schedule-form").serialize());
        
        posting.done(function(data) {
                alert("testing");
                console.log(data);
                $("#content").empty.append("<h3>shit has changed</h3>");
            });
        posting.fail(function(data) {
            alert("Failed");
            console.log(data);
            console.log("Failed");
            Result.init();
            Result.generate();
        });
    });
}

Classes.expand = function() {
    var $formgroup = $("#classes-submit")
    var $input = $(document.createElement("input"));
    $input.attr("type", "text")
          .attr("class", "form-control")
          .attr("id", "class" + Classes.input_id.toString())
          .attr("placeholder", "Enter Class");
    ++Classes.input_id;
    $formgroup.append($input);
}
