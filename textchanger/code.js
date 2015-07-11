(function($) {
  var send_bit = function(value) {
    console.log("called send_bit(", value, ")");

    var deferred = $.Deferred();
    var blink = $("#wavetext-blink");

    blink.show();
    window.setTimeout(function() {
      blink.hide();
    }, (value) ? 80 : 40)

    //resolve after 120ms
    window.setTimeout(function() {
      deferred.resolve();
    }, 120);
    return deferred.promise();
  }

  var send_chr = function(chr) {
    console.log("called send_chr", chr);
    var deferred = $.Deferred();

    send_bit((chr.charCodeAt(0) >> 7) & 1).then(function() {
      send_bit((chr.charCodeAt(0) >> 6) & 1).then(function() {
        send_bit((chr.charCodeAt(0) >> 5) & 1).then(function() {
          send_bit((chr.charCodeAt(0) >> 4) & 1).then(function() {
            send_bit((chr.charCodeAt(0) >> 3) & 1).then(function() {
              send_bit((chr.charCodeAt(0) >> 2) & 1).then(function() {
                send_bit((chr.charCodeAt(0) >> 1) & 1).then(function() {
                  send_bit((chr.charCodeAt(0) >> 0) & 1).then(function() {
                    deferred.resolve();
                  })})})})})})})});

    return deferred.promise();
  }

  var send_str = function(str) {
    console.log("called send_str", str);
    var deferred = $.Deferred();

    var first = str.substr(0, 1);
    var rest = (str.length > 1) ? str.substr(1, str.length -1): '';

    send_chr(first).then(function() {
      if(rest)
        send_str(rest).then(function() {
          deferred.resolve();
        })
      else {
        deferred.resolve();
      }
    })

    return deferred.promise();
  }

  $(document).ready(function() {
    $("#wavetext-blink").hide();
    $("#wavetext-send").click(function() {
      send_str($("#wavetext-message").val());
    });
  });
})(jQuery);
