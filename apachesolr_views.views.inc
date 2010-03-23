<?php
// $Id$

/*
 * Load files with base classes of the contained classes.
 */

/**
 * Implementation of hook_views_handlers().
 */
function apachesolr_views_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'apachesolr_views') . '/handlers',
    ),
    'handlers' => array(
      'apachesolr_views_handler_argument_optionwidget' => array(
        'parent' => 'apachesolr_views_handler_argument',
      ),
      'apachesolr_views_handler_argument_node_type' => array(
        'parent' => 'apachesolr_views_handler_argument',
      ),
      'apachesolr_views_handler_argument' => array(
        'parent' => 'views_handler_argument',
      ),
      'apachesolr_views_handler_argument_mlt' => array(
        'parent' => 'views_handler_argument_numeric',
      ),
      'apachesolr_views_handler_argument_tid' => array(
        'parent' => 'apachesolr_views_handler_argument',
      ),
      'apachesolr_views_handler_field' => array(
        'parent' => 'views_handler_field_node',
      ),
      'apachesolr_views_handler_field_author' => array(
        'parent' => 'apachesolr_views_handler_field',
      ),
      'apachesolr_views_handler_field_date' => array(
        'parent' => 'apachesolr_views_handler_field',
      ),
      'apachesolr_views_handler_field_taxonomy' => array(
        'parent' => 'apachesolr_views_handler_field',
      ),
      'apachesolr_views_handler_field_type' => array(
        'parent' => 'apachesolr_views_handler_field',
      ),
      'apachesolr_views_handler_field_score' => array(
        'parent' => 'views_handler_field_numeric',
      ),
      'apachesolr_views_handler_field_snippet' => array(
        'parent' => 'views_handler_field',
      ),
      'apachesolr_views_handler_optionwidget_filter' => array(
        'parent' => 'views_handler_filter',
      ),
      'apachesolr_views_handler_filter_author' => array(
        'parent' => 'views_handler_filter_user_name',
      ),
      'apachesolr_views_handler_filter_search' => array(
        'parent' => 'views_handler_filter_search',
      ),
      'apachesolr_views_handler_filter_type' => array(
        'parent' => 'views_handler_filter_in_operator',
      ),
      'apachesolr_views_handler_filter_single' => array(
        'parent' => 'views_handler_filter_string',
      ),
      'apachesolr_views_handler_filter_node_access' => array(
        'parent' => 'views_handler_filter_node_access',
      ),
      'apachesolr_views_handler_filter_boolean' => array(
        'parent' => 'views_handler_filter_boolean_operator',
      ),
      'apachesolr_views_handler_sort' => array(
        'parent' => 'views_handler_sort',
      ),
    ),
  );
}

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
    'cache' => array(
      'apachesolr-time' => array(
        'path' => drupal_get_path('module', 'apachesolr_views') . '/plugins',
        'title' => t('Time-based Solr Cache'),
        'help' => t('Simple time-based caching of data extension for Solr Views'),
        'handler' => 'apachesolr_views_plugin_cache_time',
        'uses options' => TRUE,
        'base' => array('apachesolr'),
        'parent' => 'time',
        //'help topic' => 'apachesolr-cache-time', // TODO!!!
      ),
    ),
  );

}

/**
 * Implementation of hook_views_data().
 */
function apachesolr_views_views_data() {
  $data['apachesolr']['table']['group']  = t('Apache Solr');

  $data['apachesolr']['table']['base'] = array(
    'query class' => 'apachesolr_views_query',
    'title' => t('Apache Solr'),
    'help' => t('Searches the site with the Apache Solr search engine.'),
    'field' => 'nid',
  );

  $data['apachesolr']['nid'] = array(
    'title' => t('Nid'),
    'help' => t('The node ID of the node.'),
    'field' => array(
      'name field' => 'title',
      'numeric' => TRUE,
      'handler' => 'views_handler_field_node',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'apachesolr_views_handler_sort',
    ),
  );
  $data['apachesolr']['title'] = array(
    'title' => t('Title'),
    'help' => t('The title of the node.'),
    'argument' => array(
      'handler' => 'apachesolr_views_handler_argument',
    ),
    'field' => array(
      'handler' => 'views_handler_field_node',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'apachesolr_views_handler_sort',
    ),
  );
  $data['apachesolr']['created'] = array(
    'title' => t('Creation date'),
    'help' => t('The date the node was created.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'apachesolr_views_handler_sort',
    ),
  );
  $data['apachesolr']['changed'] = array(
    'title' => t('Updated date'),
    'help' => t('The date the node was last updated.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'apachesolr_views_handler_sort',
    ),
  );
  $data['apachesolr']['type'] = array(
    'title' => t('Type'),
    'help' => t('The type of a node (for example, "blog entry", "forum post", "story", etc).'),
    'argument' => array(
      'handler' => 'apachesolr_views_handler_argument_node_type',
    ),
    'field' => array(
      'handler' => 'views_handler_field_node_type',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'apachesolr_views_handler_filter_type',
    ),
    'sort' => array(
      'handler' => 'apachesolr_views_handler_sort',
    ),
  );
  $data['apachesolr']['name'] = array(
    'title' => t('Author'),
    'help' => t("The node's author."),
    'field' => array(
      'handler' => 'views_handler_field_user',
      'click sortable' => TRUE,
      'additional fields' => array('uid'),
    ),
    'sort' => array(
      'handler' => 'apachesolr_views_handler_sort',
    ),
  );
  $data['apachesolr']['uid'] = array(
    'title' => t('Author Uid'),
    'help' => t("The node's author's user ID."),
    'argument' => array(
      'handler' => 'apachesolr_views_handler_argument',
    ),
    'field' => array(
      'numeric' => TRUE,
      'handler' => 'views_handler_field_user',
      'click sortable' => TRUE,
    ),
  );
  $data['apachesolr']['body'] = array(
    'title' => t('Body'),
    'help' => t("The node's content."),
    'field' => array(
      'handler' => 'views_handler_field_markup',
      'format' => 'format',
    ),
  );
  $data['apachesolr']['comment_count'] = array(
    'title' => t('Comment count'),
    'help' => t('The number of comments that were posted to the node.'),
    'field' => array(
      'numeric' => TRUE,
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'apachesolr_views_handler_sort',
    ),
  );
  // TODO Get taxonomy fields to work
  if (module_exists('taxonomy')) {
    $data['apachesolr']['tid'] = array(
      'title' => t('Taxonomy terms'),
      'help' => t('Taxonomy terms associated with the node.'),
      'argument' => array(
        'handler' => 'apachesolr_views_handler_argument_tid',
      ),
    );
  }
  // @TODO: handle language field.
  
  $data['apachesolr']['text'] = array(
    'title' => t('Search'),
    'help' => t('Searches the content with Solr'),
    'filter' => array(
      'handler' => 'apachesolr_views_handler_filter_search',
    ),
  );
  
  // score field. Useful for when combining sorts. So you sort by score, creation etc.
  $data['apachesolr']['score'] = array(
    'title' => t('Search Score'),
    'help' => t('The score of the search for this item. When no sorts are present this is the default sort'),
    'sort' => array(
      'handler' => 'apachesolr_views_handler_sort',
    ),
    'field' => array(
      'handler' => 'apachesolr_views_handler_field_score'
    ),
  );
  
  // comment_count field useful for sorting
  $data['apachesolr']['comment_count'] = array(
    'title' => t('Comment Count'),
    'help' => t('The number of comments on a node'),
    'sort' => array(
      'handler' => 'apachesolr_views_handler_sort',
    ),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => FALSE,
    ),
  );
  
  // snippet field
  $data['apachesolr']['snippet'] = array(
    'title' => t('Search Snippet'),
    'help' => t('The return snippet that matches the query sent to Solr.'),
    'field' => array(
      'handler' => 'apachesolr_views_handler_field_snippet',
    ),
  );
  $data['apachesolr']['mlt'] = array(
    'title' => t('More Like This Node id'),
    'help' => t('Do a more like this query based on the provided nid'),
    'argument' => array(
      'handler' => 'apachesolr_views_handler_argument_mlt',
    ),
  );
  
  // node access
  if (module_exists('apachesolr_nodeaccess')) {
    $data['apachesolr']['access'] = array(
      'title' => t('Node Access Control'),
      'help' => t('Return results from Apache Solr that respect node access'),
      'filter' => array(
        'handler' => 'apachesolr_views_handler_filter_node_access',
      ),
    );
  }
  
  $data['apachesolr']['promote'] = array(
    'title' => t('Promoted to Front Page'),
    'help' => t('Whether or not this node was promoted to the front page'),
    'filter' => array(
      'handler' => 'apachesolr_views_handler_filter_boolean',
      'label' => t('Promoted'),
      'type' => 'yes-no',
    ),
  );
  
  // provide CCK mappings filters
  foreach (apachesolr_cck_fields() as $name => $field) {
    // TODO: can we make use of CCK's mapping already and just override
    // the query part???
    $options = content_allowed_values(content_fields($field['field_name']));
    if (!empty($options)) {
      $data['apachesolr'][apachesolr_index_key($field)] = array(
        'title' => t($field['label']),
        'help' => t('CCK Mapping for @fieldname', array('@fieldname' => $field['field_name'])),
        'filter' => array(
          'handler' => 'apachesolr_views_handler_optionwidget_filter',
          'cck_field' => $field,
        ),
        'argument' => array(
          'handler' => 'apachesolr_views_handler_argument_optionwidget',
          'cck_field' => $field,
        ),
      );
    }
  }
  
  return $data;
}

/**
 * Implementation of hook_views_plugins_alter().
 */
function apachesolr_views_plugins_alter(&$plugins) {
  // Add apachesolr to the base of the node row plugin.
  $plugins['row']['node']['base'][] = 'apachesolr';

  // Add apachesolr to the base of the node_rss row plugin.
  $plugins['row']['node_rss']['base'][] = 'apachesolr';
}