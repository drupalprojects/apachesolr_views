<?php

/*
 * Load files with base classes of the contained classes.
 */

/**
 * Implementation of hook_views_plugins().
 */
function apachesolr_views_views_plugins() {
  return array(
    'module' => 'apachesolr_views',
    'query' => array(
      'apachesolr_views_query' => array(
        'title' => t('Apache Solr Query'),
        'help' => t('Query that allows you to search with Apache Solr.'),
        'handler' => 'apachesolr_views_query',
        'parent' => 'views_query',
      ),
    ),
  );

}

/**
 * Implementation of hook_views_data().
 */
function apachesolr_views_views_data() {
    $name = apachesolr_default_environment();
    $apachesolr_base_table = 'apachesolr_' . $name;

    $data[$apachesolr_base_table]['table']['group'] = t('Apache Solr');
    $data[$apachesolr_base_table]['table']['base'] = array(
      'query class' => 'apachesolr_views_query',
      'title' => t('Apache Solr @name', array('@name' => $name)),
      'help' => t('Searches the site with the Apache Solr search engine for @name', array('@name' => $name)),
    );

    // Get the list of the fields in idex directly from Solr.
    try {
      $solr = apachesolr_get_solr();
      $solr_fields = $solr->getFields(1);
    }
    catch (Exception $e) {
      $solr_fields = array();
      watchdog('Apache Solr Views', nl2br(check_plain($e->getMessage())), NULL, WATCHDOG_ERROR);
    }

    foreach ($solr_fields as $solr_field_name => $solr_field) {

      // We do not allow to display 'sort_*' fields.
      if (strpos($solr_field_name, 'sort_') === 0) {
        continue;
      }

      $field_type = $solr_field->type;

      $data[$apachesolr_base_table][$solr_field_name] = array(
        'title' => $solr_field_name,
        'help' => $solr_field_name,
        'field' => array(
          'handler' => 'apachesolr_views_handler_field',
          'click sortable' => TRUE,
        ),
        'filter' => array(
          'handler' => 'apachesolr_views_handler_filter',
        ),
        'sort' => array(
          'handler' => 'apachesolr_views_handler_sort',
        ),
      );
    }

  return $data;
}
