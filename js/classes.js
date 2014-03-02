var Classes = Classes || {};

Classes.input_id = 1;

Classes.init = function() {
    var $formgroup = $("#classes-submit")

    for (var i = 0; i < 5; ++i) {
        var $input = $(document.createElement("input"));
        $input.attr("type", "text")
              .attr("class", "form-control")
              .attr("id", "class" + Classes.input_id.toString())
              .attr("placeholder", "Enter Class");
        ++Classes.input_id;
        $formgroup.append($input);
    }
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
