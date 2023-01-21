$(document).ready(function(){
    $(".act").click(function(event){
        $(this).parent().parent().find(".ham").toggleClass("active");
    });
});