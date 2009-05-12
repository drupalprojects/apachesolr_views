<?php

// $Id$

/**
 * @file
 * provides a field handler for the snippet coming from the ApacheSolr search engine
 */
class apachesolr_views_handler_field_snippet extends views_handler_field {
  function query() {
    // Wierd that with ht.alternateField we can't have the snippet be the body...
    $this->query->add_field('body');
  }
  
  function render($doc) {
    $response = apachesolr_static_response_cache();
    $hl_fl = $this->query->get_param('hl.fl');
    if (is_null($hl_fl)) {
      $hl_fl = 'body';
    }
    $snippet = isset($response->highlighting->{$doc->id}->$hl_fl) ? theme('apachesolr_search_snippets', $doc, $response->highlighting->{$doc->id}->$hl_fl) : $doc->body;
    
    return $snippet;
  }
}