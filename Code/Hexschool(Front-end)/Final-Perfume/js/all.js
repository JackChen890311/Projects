$(document).ready(function(){
    $(".act").click(function(event){
        $(this).parent().parent().find(".ham").toggleClass("d-none");
        $(this).parent().parent().find(".ham").toggleClass("d-block");
    });
});