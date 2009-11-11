<?php

/* $Id$ */

/**
 * Class that allows searching the site with Apache Solr through a view.
 */
class apachesolr_views_handler_filter_search extends views_handler_filter_search {
  
  public function query() {
    if (!empty($this->value)) {
      $this->query->set_query($this->value);
    }
  }
  
   /**
   * Validate the options form.
   */
  function exposed_validate($form, &$form_state) {
    // @TODO decide what we want to do here if anything
  }
  
}
